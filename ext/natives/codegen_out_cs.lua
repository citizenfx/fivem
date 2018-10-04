local function printFunctionName(native)
	return native.name:lower():gsub('0x', 'n_0x'):gsub('_(%a)', string.upper):gsub('(%a)(.+)', function(a, b)
		return a:upper() .. b
	end)
end

-- C# language words
local langWords = {
	['abstract'] = '_abstract',
	['as'] = '_as',
	['base'] = '_base',
	['bool'] = '_bool',
	['break'] = '_break',
	['byte'] = '_byte',
	['case'] = '_case',
	['catch'] = '_catch',
	['char'] = '_char',
	['checked'] = '_checked',
	['class'] = '_class',
	['const'] = '_const',
	['continue'] = '_continue',
	['decimal'] = '_decimal',
	['default'] = '_default',
	['delegate'] = '_delegate',
	['do'] = '_do',
	['double'] = '_double',
	['else'] = '_else',
	['enum'] = '_enum',
	['event'] = '_event',
	['explicit'] = '_explicit',
	['extern'] = '_extern',
	['false'] = '_false',
	['finally'] = '_finally',
	['fixed'] = '_fixed',
	['float'] = '_float',
	['for'] = '_for',
	['foreach'] = '_foreach',
	['goto'] = '_goto',
	['if'] = '_if',
	['implicit'] = '_implicit',
	['in'] = '_in',
	['int'] = '_int',
	['interface'] = '_interface',
	['internal'] = '_internal',
	['is'] = '_is',
	['lock'] = '_lock',
	['long'] = '_long',
	['namespace'] = '_namespace',
	['new'] = '_new',
	['null'] = '_null',
	['object'] = '_object',
	['operator'] = '_operator',
	['out'] = '_out',
	['override'] = '_override',
	['params'] = '_params',
	['private'] = '_private',
	['protected'] = '_protected',
	['public'] = '_public',
	['readonly'] = '_readonly',
	['ref'] = '_ref',
	['return'] = '_return',
	['sbyte'] = '_sbyte',
	['sealed'] = '_sealed',
	['short'] = '_short',
	['sizeof'] = '_sizeof',
	['stackalloc'] = '_stackalloc',
	['static'] = '_static',
	['string'] = '_string',
	['struct'] = '_struct',
	['switch'] = '_switch',
	['this'] = '_this',
	['throw'] = '_throw',
	['true'] = '_true',
	['try'] = '_try',
	['typeof'] = '_typeof',
	['uint'] = '_uint',
	['ulong'] = '_ulong',
	['unchecked'] = '_unchecked',
	['unsafe'] = '_unsafe',
	['ushort'] = '_ushort',
	['using'] = '_using',
	['virtual'] = '_virtual',
	['void'] = '_void',
	['volatile'] = '_volatile',
	['while'] = '_while',
	['add'] = '_add',
	['alias'] = '_alias',
	['ascending'] = '_ascending',
	['async'] = '_async',
	['await'] = '_await',
	['descending'] = '_descending',
	['dynamic'] = '_dynamic',
	['from'] = '_from',
	['get'] = '_get',
	['global'] = '_global',
	['group'] = '_group',
	['into'] = '_into',
	['join'] = '_join',
	['let'] = '_let',
	['nameof'] = '_nameof',
	['orderby'] = '_orderby',
	['remove'] = '_remove',
	['select'] = '_select',
	['set'] = '_set',
	['value'] = '_value',
	['var'] = '_var',
	['when'] = '_when',
	['where'] = '_where',
	['yield'] = '_yield'
}

local usedNatives = {}

-- sort the natives table
local _natives = {}

for _, v in ipairs(natives) do
	table.insert(_natives, v)
end

table.sort(_natives, function(a, b)
	return printFunctionName(a) < printFunctionName(b)
end)

local function printReturnType(type)
	if type.nativeType == 'string' then
		return 'string'
	elseif type.nativeType == 'bool' then
		return 'bool'
	elseif type.nativeType == 'float' then
		return 'float'
	elseif type.nativeType == 'vector3' then
		return 'Vector3'
	elseif type.nativeType == 'int' then
		if type.subType == 'long' then
			return 'long'
		else
			return 'int'
		end
	elseif type.nativeType == 'object' then
		return 'dynamic'
	else
		return 'int'
	end
end

local function trimAndNormalize(str)
	return trim(str):gsub('/%*', ' -- [['):gsub('%*/', ']] '):gsub('&', '&amp;'):gsub('<', '&lt;'):gsub('>', '&gt;')
end

local function formatDocString(native)
	local t = '\t\t'
	local d = parseDocString(native)

	if not d then
		return ''
	end

	local firstLine, nextLines = d.summary:match("([^\n]+)\n?(.*)")

	if not firstLine then
		return ''
	end

	local l = t .. '/// <summary>\n'
	l = l .. t .. '/// ' .. trimAndNormalize(firstLine) .. "\n"
	for line in nextLines:gmatch("([^\n]+)") do
		l = l ..t .. '/// ' .. trimAndNormalize(line) .. "\n"
	end
	l = l .. t .. '/// </summary>\n'

	if d.hasParams then
		for _, v in ipairs(d.params) do
			l = l ..t .. '/// <param name="' .. v[1] .. '">' .. trimAndNormalize(v[2]) .. '</param>\n'
		end
	end

	if d.returns then
		l = l ..t .. '/// <returns>' .. trimAndNormalize(d.returns) .. '</returns>\n'
	end

	return l
end

local function parseArgument(argument, native)
	local argType

	if argument.type.name == 'func' then
		argType = 'InputArgument'
	elseif argument.type.name == 'Hash' then
		argType = 'uint'
	elseif argument.type.name == 'uint' then
		argType = 'uint'
	elseif argument.type.nativeType == 'Any*' then
		argType = 'object'
	elseif argument.type.nativeType == 'string' then
		argType = 'string'
	elseif argument.type.nativeType == 'int' then
		if argument.type.subType == 'long' then
			argType = 'long'
		else
			argType = 'int'
		end
	elseif argument.type.nativeType == 'float' then
		argType = 'float'
	elseif argument.type.nativeType == 'bool' then
		argType = 'bool'
	elseif argument.type.nativeType == 'vector3' then
		argType = 'Vector3'
	end

	local name = argument.name

	-- cs's reserved word
	if langWords[name] then
		name = langWords[name]
	end

	return name, argType
end

local function formatArgs(native)
	local args = {}
	local argsDefs = {}
	local nativeArgs = {}

	if native.arguments then
		for _, argument in pairs(native.arguments) do
			local argumentName, argType = parseArgument(argument, native)

			table.insert(args, { argumentName, argType, argument.pointer })
			table.insert(nativeArgs, argumentName)

			if argument.pointer then
				table.insert(argsDefs, 'ref ' .. argType .. ' ' .. argumentName)
			else
				table.insert(argsDefs, argType .. ' ' .. argumentName)
			end
		end
	end
	
	return args, argsDefs, nativeArgs
end

local function formatWrapper(native, fnName)
	local t = '\t\t\t'
	local body = ''
	
	local args, argsDefs, nativeArgs = formatArgs(native)
		
	local argNames = {}
	
	for argn, arg in pairs(args) do
		local name, type, ptr = table.unpack(arg)
		
		if ptr then
			name = 'ref ' .. name
		end
		
		table.insert(argNames, name)
	end

	body = body .. '(' .. table.concat(argsDefs, ', ') .. ')\n'
	body = body .. '\t\t{\n'
	
	local retType = 'void'
	if native.returns then
		retType = printReturnType(native.returns)
	end
	
	body = body .. t
	
	if retType ~= 'void' then
		body = body .. 'return '
	end
	
	body = body .. fnName .. '(' .. table.concat(argNames, ', ') .. ');\n'
	
	body = body .. '\t\t}\n\n'
	
	return body
end

local function formatImpl(native)
	local t = '\t\t\t'
	local body = ''
	
	local args, argsDefs, nativeArgs = formatArgs(native)

	body = body .. '(' .. table.concat(argsDefs, ', ') .. ')\n'
	body = body .. '\t\t{\n'

	local retType = 'void'
	if native.returns then
		retType = printReturnType(native.returns)
	end

	-- First lets make output args containers if needed
	local refValNum = 0
	local refToArg = {}
	for argn, arg in pairs(args) do
		local name, type, ptr = table.unpack(arg)

		if ptr == true then
			if type == 'Vector3' then
				type = 'NativeVector3'
			end
		
			refValNum = refValNum + 1
			local refName = 'ref_' .. name

			refToArg[refName] = { name, type }
			nativeArgs[argn] = refName

			body = body .. t .. type .. ' ' .. refName .. ' = ' .. name .. ';\n'
		end
	end
	if refValNum > 1 then
		body = body .. '\n'
	end
	
	local appendix = ''
	
	--body = body .. t .. 'using (var scriptContext = new ScriptContext())\n'
	body = body .. t .. '{\n'
	body = body .. t .. '\tvar scriptContext = new ScriptContext();\n'
	
	local fastEligible = true
	
	for argn, arg in pairs(args) do
		local _, type = table.unpack(arg)
		
		if type == 'string' or type == 'InputArgument' or type == 'object' then
			fastEligible = false
		end
	end
	
	if not fastEligible then
		for argn, arg in pairs(args) do
			local name, type, ptr = table.unpack(arg)

			if ptr then
				body = body .. t .. '\tscriptContext.PushFast(new System.IntPtr(&ref_' .. name .. '));\n'
				appendix = appendix .. t .. '\t' .. name .. ' = ref_' .. name .. ';\n'
			elseif type == 'string' then
				body = body .. t .. '\tscriptContext.PushString(' .. name .. ');\n'
			elseif type == 'InputArgument' or type == 'object' then
				body = body .. t .. '\tscriptContext.Push(' .. name .. ');\n'
			else
				body = body .. t .. '\tscriptContext.PushFast(' .. name .. ');\n'
			end
		end
	else
		body = body .. t .. '\tfixed (byte* functionData = ScriptContext.m_context.functionData)\n'
		body = body .. t .. '\t{\n'
		
		local numArgs = 0
		
		for argn, arg in pairs(args) do
			local name, type, ptr = table.unpack(arg)
			
			local val = name
			
			if ptr then
				type = 'System.IntPtr'
				val = 'new System.IntPtr(&ref_' .. name .. ')'
				
				appendix = appendix .. t .. '\t' .. name .. ' = ref_' .. name .. ';\n'
			end
			
			body = body .. t .. '\t\t*(long*)(&functionData[' .. (8 * numArgs) .. ']) = 0;\n'
			
			body = body .. t .. '\t\t*(' .. type .. '*)(&functionData[' .. (8 * numArgs) .. ']) = ' .. val .. ';\n'
			
			if type == 'Vector3' then
				numArgs = numArgs + 3
			else
				numArgs = numArgs + 1
			end
		end
		
		body = body .. t .. '\t}\n\n'
		
		body = body .. t .. '\tScriptContext.m_context.numArguments = ' .. numArgs .. ';\n'
	end
	
	body = body .. t .. '\tscriptContext.Invoke((ulong)' .. native.hash .. ');\n'
	
	if retType ~= 'void' then
		local tempRetType = retType
	
		if retType == 'string' or retType == 'object' or retType == 'dynamic' then
			if retType == 'dynamic' then
				tempRetType = 'object'
			end
		
			appendix = appendix .. t .. '\treturn (' .. retType .. ')scriptContext.GetResult(typeof(' .. tempRetType .. '));\n'
		else
			if retType == 'Vector3' then
				tempRetType = 'NativeVector3'
			end
		
			appendix = appendix .. t .. '\treturn scriptContext.GetResultFast<' .. tempRetType .. '>();\n'
		end
	end
	
	appendix = appendix .. t .. '}\n'

	return retType, body .. appendix .. '\t\t}\n'
end

local function printNative(native)
	local nativeName = printFunctionName(native)
	local appendix = ''

	if usedNatives[nativeName] then
		appendix = (usedNatives[nativeName] + 1) .. ''
		usedNatives[nativeName] = usedNatives[nativeName] + 1
	else
		usedNatives[nativeName] = 1
	end
	
	local baseAppendix = appendix

	local doc = formatDocString(native)
	local retType, def = formatImpl(native)
	local wrapper = formatWrapper(native, 'Internal' .. nativeName .. baseAppendix)

	local str = string.format("%s\t\t[System.Security.SecuritySafeCritical]\n\t\tpublic static %s %s%s", doc, retType, nativeName .. appendix, wrapper)

	for _, alias in ipairs(native.aliases) do
		local aliasName = printFunctionName({ name = alias })
		
		if usedNatives[aliasName] then
			appendix = (usedNatives[aliasName] + 1) .. ''
			usedNatives[aliasName] = usedNatives[aliasName] + 1
		else
			usedNatives[aliasName] = 1
		end

		if nativeName ~= aliasName then
			str = str .. string.format("%s\t\t[System.Security.SecuritySafeCritical]\n\t\tpublic static %s %s%s", doc, retType, aliasName .. appendix, wrapper)
		end
	end
	
	str = str .. string.format("\t\t[System.Security.SecurityCritical]\n\t\tprivate static unsafe %s Internal%s%s", retType, nativeName .. baseAppendix, def)

	return str
end

-- print('using System;')
-- print('using CitizenFX.Core;')
-- print('using CitizenFX.Core.Native;')
-- print('')
-- print('namespace CitizenFX.Core.Native\n{')
print('\tpublic static class API\n\t{')

for _, v in pairs(_natives) do
	if matchApiSet(v) then
		print(printNative(v))
	end
end

print('\t}')
-- print('}')
