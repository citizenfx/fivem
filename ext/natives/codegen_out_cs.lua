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

local function wrapLines(str, openTag, closeTag, allowEmptyTag)
	local firstLine, nextLines = str:match("([^\n]+)\n?(.*)")

	local t = '\t\t'
	if firstLine then
		local l = t .. '/// ' .. openTag .. '\n'
		l = l .. t .. '/// ' .. trimAndNormalize(firstLine) .. "\n"
		for line in nextLines:gmatch("([^\n]+)") do
			l = l ..t .. '/// ' .. trimAndNormalize(line) .. "\n"
		end
		l = l .. t .. '/// ' .. closeTag .. '\n'
		return l
	elseif allowEmptyTag then
		return t .. '/// ' .. openTag:sub(1, openTag:len() - 1) .. ' />\n'
	else
		return ''
	end	
end

local function formatDocString(native)
	local d = parseDocString(native)

	if not d then
		return ''
	end
	
	local l = ''
	l = l .. wrapLines(d.summary, '<summary>', '</summary>')

	if d.hasParams then
		for _, v in ipairs(d.params) do
			l = l .. wrapLines(v[2], '<param name="' .. (langWords[v[1]] or v[1]) .. '">', '</param>', true)
		end
	end

	if d.returns then
		l = l .. wrapLines(d.returns, '<returns>', '</returns>')
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

local function formatImpl(native, baseAppendix)
	local t = '\t\t\t'
	local body = ''
	
	local nativeName = printFunctionName(native) .. baseAppendix
	local args, argsDefs, nativeArgs = formatArgs(native)

	body = body .. '(' .. table.concat(argsDefs, ', ') .. ')\n'
	body = body .. '\t\t{\n'

	local retType = 'void'
	if native.returns then
		retType = printReturnType(native.returns)
	end

	-- First lets make output args containers if needed
	local hyperDriveSafe = true
	
	local refValNum = 0
	local refToArg = {}
	for argn, arg in pairs(args) do
		local name, type, ptr = table.unpack(arg)

		if ptr == true then
			if type == 'Vector3' then
				hyperDriveSafe = false
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
	
	body = body .. t .. '{\n'
	--body = body .. t .. '\tScriptContext.Reset();\n'
	
	body = body .. t .. '\tbyte* cxtBytes = stackalloc byte[sizeof(ContextType)];\n'
	body = body .. t .. '\tContextType* cxt = (ContextType*)cxtBytes;\n'
	
	local fastEligible = true
	local pushFree = true
	
	for argn, arg in pairs(args) do
		local _, type = table.unpack(arg)
		
		if type == 'InputArgument' or type == 'object' then
			fastEligible = false
			pushFree = false
		end
		
		if type == 'string' then
			pushFree = false
		end
	end
	
	if not pushFree and not fastEligible then
		body = body .. t .. '\tcxt->numArguments = 0;\n'
		body = body .. t .. '\tcxt->numResults = 0;\n'
	end
	
	body = body .. t .. '\tlong* _fnPtr = (long*)&cxt->functionData[0];\n'
	
	if not fastEligible then
		for argn, arg in pairs(args) do
			local name, type, ptr = table.unpack(arg)

			if ptr then
				body = body .. t .. '\tScriptContext.PushFast(cxt, new System.IntPtr(&ref_' .. name .. '));\n'
				appendix = appendix .. t .. '\t' .. name .. ' = ref_' .. name .. ';\n'
			elseif type == 'string' then
				body = body .. t .. '\tScriptContext.PushString(cxt, ' .. name .. ');\n'
			elseif type == 'InputArgument' or type == 'object' then
				body = body .. t .. '\tScriptContext.Push(cxt, ' .. name .. ');\n'
			else
				body = body .. t .. '\tScriptContext.PushFast(cxt, ' .. name .. ');\n'
			end
		end
	else
		local numArgs = 0
		
		for argn, arg in pairs(args) do
			local name, type, ptr = table.unpack(arg)
			
			local val = name
			
			if ptr then
				type = 'System.IntPtr'
				val = 'new System.IntPtr(&ref_' .. name .. ')'
				
				appendix = appendix .. t .. '\t' .. name .. ' = ref_' .. name .. ';\n'
			end
			
			if type == 'int' then
				body = body .. t .. '\t_fnPtr[' .. numArgs .. '] = (long)' .. val .. ';\n'
			--elseif type == 'bool' then
			--	body = body .. t .. '\tfnPtr[' .. numArgs .. ']) = (long)(' .. val .. ' ? 1 : 0);\n'
			elseif type == 'string' then
				body = body .. t .. '\tcxt->numArguments = ' .. tostring(argn - 1) .. ';\n'
				body = body .. t .. '\tScriptContext.PushString(cxt, ' .. name .. ');\n'
			else
				-- assuming float is safe as only doing 32 bit reads?
				if type ~= 'float' and type ~= 'System.IntPtr' then
					body = body .. t .. '\t_fnPtr[' .. numArgs .. '] = 0;\n'
				end
			
				body = body .. t .. '\t*(' .. type .. '*)(&_fnPtr[' .. numArgs .. ']) = ' .. val .. ';\n'
			end
			
			if type == 'Vector3' then
				numArgs = numArgs + 3
			else
				numArgs = numArgs + 1
			end
		end
		
		if native.ns == 'CFX' then
			body = body .. t .. '\tcxt->numArguments = ' .. numArgs .. ';\n'
		end
	end
	
	if hyperDriveSafe then
		body = body .. "\n#if !USE_HYPERDRIVE\n"
	end
	
	body = body .. t .. '\tScriptContext.Invoke(cxt, (ulong)' .. native.hash .. ');\n'
	
	if hyperDriveSafe then
		body = body .. "#else\n"
		body = body .. t .. '\tcxt->functionDataPtr = _fnPtr;\n'
		body = body .. t .. '\tcxt->retDataPtr = _fnPtr;\n'
		body = body .. t .. ("\tvar invv = m_invoker%s;\n"):format(nativeName)
		body = body .. t .. ("\tif (invv == null) m_invoker%s = invv = ScriptContext.DoGetNative(%s);\n"):format(nativeName, native.hash)
		body = body .. t .. ("\tif (!invv(cxt)) { throw new System.Exception(\"Native invocation failed.\"); }\n")
		body = body .. "#endif\n"
	end
	
	if retType ~= 'void' then
		local tempRetType = retType
	
		if retType == 'string' or retType == 'object' or retType == 'dynamic' then
			if retType == 'dynamic' then
				tempRetType = 'object'
			end
		
			appendix = appendix .. t .. '\treturn (' .. retType .. ')ScriptContext.GetResult(cxt, typeof(' .. tempRetType .. '));\n'
		else
			if retType == 'Vector3' then
				tempRetType = 'NativeVector3'
			end

			appendix = appendix .. '\n'
			appendix = appendix .. t .. '\treturn *(' .. tempRetType .. '*)(&cxt->functionData[0]);\n'
		end
	end
	
	appendix = appendix .. t .. '}\n'

	return retType, (body .. appendix .. '\t\t}\n'), hyperDriveSafe
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
	local retType, def, hyperDriveSafe = formatImpl(native, baseAppendix)
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
	if hyperDriveSafe then
		str = str .. string.format("\n#if USE_HYPERDRIVE\n\t\tprivate static ScriptContext.CallFunc m_invoker%s;\n#endif\n", nativeName .. baseAppendix);
	end	
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
