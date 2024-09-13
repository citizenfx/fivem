'''
Script to help with generating and validating native declaration files based on existing C++ implementations.

The script statically analyzes C++ files on the best efforts basis. It retrieves native name, its arguments
and return type (based on "SetResult", "GetArgument" etc calls). The script is limited in its capabilities
and may not be able to retrieve all the information in some cases. Known limitations are:
- Natives that pass context to functions outside of the current file. If such functions use context to get
    arguments or set result, the script won't be able to retrieve this information.
- Natives that pass context to lambdas or other callable objects other than functions. If such functions use
    context to get arguments or set result, the script won't be able to retrieve this information.
- "SetResult" calls without template argument. The script will not be able to deduct return type in such case
    and assume that return type is unknown (but not void).
- "GetArgument" calls that use variables instead of constants as argument. The script will ignore such arguments
    as it and won't be able to retrieve the argument number.
- Process information that is determined in runtime. E.g if return type depends on the if statement.

Feel free to update the script to improve its capabilities.

Example usage:
python .\code\tools\native_decl_util.py \
    X:\fivem\code\components\extra-natives-five\src\PedCollectionsNatives.cpp \
    X:\fivem\code\components\citizen-server-impl\src\state\ServerGameState_Scripting.cpp \
    --out X:\fivem\ext\native-decls\ \
    --validate \
    --generate-decls
'''

import argparse
import dataclasses
import glob
import itertools
import os
import re
import string


_REGISTER_NATIVE_HANDLER_CALL_REGEX = r'fx::ScriptEngine::RegisterNativeHandler\(\"(.+?)\", [\w]*?\(?\[\]\(fx::ScriptContext& (.+?)(\)|,)[\s\S]*?{'
_GET_ARGUMENT_REGEX = r'.(GetArgument|CheckArgument)<(.+?)>\((\d+)\)'
_GET_RETURN_REGEX = r'.SetResult(<.+?>|)\('
_FUNCTION_CALL_REGEX_PATTERN = r'\b([\w]+?)(<.*?>)?\(.*?%s(\)|[^\.].*?\))'
_FUNCTION_DEF_START_REGEX_PATTERN = r'(template<.+?>)?\s[\w \t\*]*? %s\([\s\S]*?fx::ScriptContext& (.+?)(\)|,)[\s\S]*?{'
_NATIVE_DECL_REGEX_PATTERN = r'```c\s(.+?) %s\((.*?)\);\s```'

_PARAMETER_DOC_TEMPLATE = string.Template('* **$VARIABLE_NAME**: ...\n')
_NATIVE_DECL_TEMPLATE = string.Template('''
---
ns: CFX
apiset: ...
game: ...
---
## $NATIVE_NAME

```c
$RETURN_TYPE $NATIVE_NAME($PARAMETERS);
```

...

## Examples

```lua
...
```

## Parameters
$PARAMETERS_DOCS

## Return value
...

''')

_ANY_TYPE = ''

_DECL_TYPE_ANALOGS = {
    'BOOL': 'bool',
    'BOOL*': 'bool*',
    'Cam': 'int',
    'Entity': 'int',
    'func': 'char*',
    'Hash': 'int',
    'Object': 'int',
    'Ped': 'int',
    'Player': 'int',
    'Vehicle': 'int',
}


@dataclasses.dataclass
class ContextUsage:
    arguments: dict[int, str]
    # None corresponds to absence of SetResult call.
    # Empty string corresponds to SetResult call without template argument.
    returnType: str|None

    def __hash__(self):
        return hash(repr(self))


def GetArgsCount(usageInfo: ContextUsage) -> int:
    return max([-1] + list(usageInfo.arguments.keys())) + 1


def GetDeclType(implType: str|None) -> str:
    if implType is None:
        return 'void'
    if implType == _ANY_TYPE:
        return 'unknown_type'

    implType = implType.removeprefix('const').strip()
    implType = implType.replace('scrVector', 'Vector3')
    for intType in ['uint32_t', 'int32_t', 'uint16_t', 'int16_t', 'uint8_t', 'int8_t']:
        implType = implType.replace(intType, 'int')

    return implType


def AreTypesEqual(declType: str, implType: str) -> bool:
    if not implType:
        return True
    if declType in _DECL_TYPE_ANALOGS:
        declType = _DECL_TYPE_ANALOGS[declType]
    return declType == GetDeclType(implType)


def ToCamelCase(snakeStr: str) -> str:
    return "".join(x.capitalize() for x in snakeStr.lower().split("_"))


def FindFunctionEnd(fileContent: str, begin: int) -> int:
    bracketBalance = 1
    for i in range(begin, len(fileContent)):
        if fileContent[i] == '{':
            bracketBalance += 1
        elif fileContent[i] == '}':
            bracketBalance -= 1

        if bracketBalance == 0:
            return i

    return -1


def MatchTemplateArgs(
    args: str|None,
    params: str|None,
    templateArgsMapping: dict[str, list[str|None]]
) -> dict[str, list[str|None]]:

    if params is None:
        return {}
    paramsList = []
    for p in params.removeprefix('template').strip('<> ').split(', '):
        if p.startswith('typename') or p.startswith('class'):
            paramsList.append(p.strip().removeprefix('typename').removeprefix('class').strip())
        else:
            paramsList.append(p.split(' ')[-1].strip())

    if args is None:
        return {p: [None] for p in paramsList}
    argsList = [a.strip() for a in args.strip('<> ').split(', ')]

    result = {}
    for i in range(len(argsList)):
        if paramsList[i].startswith('...'):
            variadicArgs = list(itertools.chain.from_iterable(templateArgsMapping.get(a, [a]) for a in argsList[i:]))
            result[paramsList[i].removeprefix('...').strip()] = variadicArgs
            break
        result[paramsList[i]] = templateArgsMapping.get(argsList[i], [argsList[i]])

    if len(argsList) < len(paramsList):
        for p in paramsList[len(argsList):]:
            result[p.removeprefix('...').strip()] = []

    return result


def GetContextUsageInfo(
    nativeName: str,
    fileContent: str,
    functionContent: str,
    contextVar: str,
    prevContextUsage: ContextUsage,
    templateArgsMapping: dict[str, list[str|None]],
) -> ContextUsage:

    result = prevContextUsage

    matches = re.findall(contextVar + _GET_ARGUMENT_REGEX, functionContent)
    for m in matches:
        for curType in templateArgsMapping.get(m[1], [m[1]]):
            prevType = result.arguments.get(int(m[2]))
            if prevType is None:
                result.arguments[int(m[2])] = curType
            elif prevType != _ANY_TYPE and prevType != curType:
                print(f'Potential error in {nativeName} native implementation. Argument {m[2]} has type mismatch. Used both as {curType} and {prevType}.')
                result.arguments[int(m[2])] = _ANY_TYPE

    matches = re.findall(contextVar + _GET_RETURN_REGEX, functionContent)
    for m in matches:
        m = m.strip('<>')
        for curType in templateArgsMapping.get(m, [m]):
            if result.returnType is None:
                result.returnType = curType
            elif result.returnType != _ANY_TYPE and result.returnType != curType:
                print(f'Potential error in {nativeName} native implementation. Return has type mismatch. Used both as {curType} and {result.returnType}.')
                result.returnType = _ANY_TYPE

    matches = re.finditer(_FUNCTION_CALL_REGEX_PATTERN % contextVar, functionContent)
    for m in matches:
        subFuncMatches = re.finditer(_FUNCTION_DEF_START_REGEX_PATTERN % m.group(1), fileContent)
        for subFuncMatch in subFuncMatches:
            endIndex = FindFunctionEnd(fileContent, subFuncMatch.span()[1])
            functionBody = fileContent[subFuncMatch.span()[1] : endIndex]
            subFuncTemplateArgsMapping = MatchTemplateArgs(m.group(2), subFuncMatch.group(1), templateArgsMapping)
            result = GetContextUsageInfo(nativeName, fileContent, functionBody, subFuncMatch.group(2), result, subFuncTemplateArgsMapping)

    return result


def CreateDecl(usageInfo: ContextUsage, nativeName: str) -> str:
    argsCount = GetArgsCount(usageInfo)
    orderedArgs = [GetDeclType(usageInfo.arguments.get(i, _ANY_TYPE)) for i in range(argsCount)]
    return _NATIVE_DECL_TEMPLATE.substitute(
        {
            'NATIVE_NAME': nativeName,
            'RETURN_TYPE': GetDeclType(usageInfo.returnType),
            'PARAMETERS': ', '.join(f'{t} arg_{i}' for i, t in enumerate(orderedArgs)),
            'PARAMETERS_DOCS': ''.join(_PARAMETER_DOC_TEMPLATE.substitute({'VARIABLE_NAME': f'arg_{i}'}) for i in range(argsCount))
        }
    )


def ValidateDecl(decl: str, usageInfo: ContextUsage, nativeName: str) -> str|None:
    m = re.search(_NATIVE_DECL_REGEX_PATTERN % nativeName, decl)
    if m is None:
        return 'Function declaration not found. Most likely either name in declaration doesn\'t match name in the implementation or ";" is missing.'
    
    returnType = m.group(1)
    if not AreTypesEqual(returnType, usageInfo.returnType):
        return f'Return type mismatch. Expected {GetDeclType(usageInfo.returnType)}, got {returnType}.'

    args = m.group(2).split(', ')
    if len(args) < GetArgsCount(usageInfo):
        return f'Argument count mismatch. Expected {GetArgsCount(usageInfo)}, got {len(args)}.'

    for i, arg in enumerate(args):
        argType = ' '.join(arg.split(' ')[:-1])
        if not AreTypesEqual(argType, usageInfo.arguments.get(i, _ANY_TYPE)):
            return f'Argument {i} type mismatch. Expected {GetDeclType(usageInfo.arguments.get(i, _ANY_TYPE))}, got {argType}.'

    return None


def ProcessDecls(fileContent: str, nativeDeclsFolder: str, validateDecls: bool, generateDecls: bool) -> None:
    matches = re.finditer(_REGISTER_NATIVE_HANDLER_CALL_REGEX, fileContent)
    for m in matches:
        nativeName = m.group(1)
        nativeDeclFile = os.path.join(nativeDeclsFolder, f'{ToCamelCase(nativeName)}.md')
        declFileExists = os.path.isfile(nativeDeclFile)
        if not declFileExists and not generateDecls:
            print(f'Declaration for native {nativeName} is not found. Expected file template location: {nativeDeclFile}.')
        
        if not generateDecls and not validateDecls:
            continue

        endIndex = FindFunctionEnd(fileContent, m.span()[1])
        functionBody = fileContent[m.span()[1] : endIndex]
        usageInfo = GetContextUsageInfo(nativeName, fileContent, functionBody, m.group(2), ContextUsage({}, None), {})
        
        if not declFileExists and generateDecls:
            print(f'Declaration for native {nativeName} is not found. Creating declaration file template: {nativeDeclFile}.')
            with open(nativeDeclFile, "w") as f:
                f.write(CreateDecl(usageInfo, nativeName))
        elif declFileExists and validateDecls:
            with open(nativeDeclFile, 'r') as f:
                validationError = ValidateDecl(f.read(), usageInfo, nativeName)
                if validationError is not None:
                    print(f'Native declaration in {nativeDeclFile} doesn\'t match the native implementation: {validationError}')


def main(files: list[str], filesFolder: str, nativeDeclsFolder: str, validateDecls: bool, generateDecls: bool) -> None:
    if filesFolder:
        for newFile in glob.glob(os.path.join(filesFolder, '**'), recursive=True):
            if newFile.endswith('.cpp'):
                files.append(newFile)

    for file in files:
        with open(file, 'r') as f:
            ProcessDecls(f.read(), nativeDeclsFolder, validateDecls, generateDecls)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('files', type=str, nargs='*', help='Files that contain native implementations.')
    parser.add_argument('--input-folder', type=str, help=(
        'Directory, where files with native implementations should be searched. '
        'Note: it will cause all .cpp files in all sub directories to be processed, which may take a while.'
    ))
    parser.add_argument('--out', type=str, help='Directory, where existing native declaration files will be searched and new ones created.')
    parser.add_argument('--validate', action='store_true', help=(
        'If flag is set - the script will also check that existing native declarations match the implementation.'
    ))
    parser.add_argument('--generate-decls', action='store_true', help=(
        'If flag is set - the script will create templates for natives that miss declarations. '
        'If not set - list of missing declarations will be printed.'
    ))

    args = parser.parse_args()
    main(args.files, args.input_folder, args.out, args.validate, args.generate_decls)
