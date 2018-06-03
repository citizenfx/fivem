local function printFunctionName(native)
	return native.name:lower():gsub('0x', 'n_0x'):gsub('_(%a)', string.upper):gsub('(%a)(.+)', function(a, b)
		return a:upper() .. b
	end)
end

-- ts's reserved words
local langWords = {
	['break'] = '_break',
	['as'] = '_as',
	['any'] = '_any',
	['case'] = '_case',
	['implements'] = '_implements',
	['boolean'] = '_boolean',
	['catch'] = '_catch',
	['interface'] = '_interface',
	['constructor'] = '_constructor',
	['class'] = '_class',
	['let'] = '_let',
	['declare'] = '_declare',
	['const'] = '_const',
	['package'] = '_package',
	['get'] = '_get',
	['continue'] = '_continue',
	['private'] = '_private',
	['module'] = '_module',
	['debugger'] = '_debugger',
	['protected'] = '_protected',
	['require'] = '_require',
	['default'] = '_default',
	['public'] = '_public',
	['number'] = '_number',
	['delete'] = '_delete',
	['static'] = '_static',
	['set'] = '_set',
	['do'] = '_do',
	['yield'] = '_yield',
	['string'] = '_string',
	['else'] = '_else',
	['symbol'] = '_symbol',
	['enum'] = '_enum',
	['type'] = '_type',
	['export'] = '_export',
	['from'] = '_from',
	['extends'] = '_extends',
	['of'] = '_of',
	['false'] = '_false',
	['finally'] = '_finally',
	['for'] = '_for',
	['function'] = '_function',
	['if'] = '_if',
	['import'] = '_import',
	['in'] = '_in',
	['instanceof'] = '_instanceof',
	['new'] = '_new',
	['null'] = '_null',
	['return'] = '_return',
	['super'] = '_super',
	['switch'] = '_switch',
	['this'] = '_this',
	['throw'] = '_throw',
	['true'] = '_true',
	['try'] = '_try',
	['typeof'] = '_typeof',
	['var'] = '_var',
	['void'] = '_void',
	['while'] = '_while',
	['with'] = '_with'
}

-- sort the natives table
local _natives = {}

for _, v in ipairs(natives) do
	table.insert(_natives, v)
end

table.sort(_natives, function(a, b)
	return printFunctionName(a) < printFunctionName(b)
end)

local function isSinglePointerNative(native)
	local foundPointer = false

	for _, v in ipairs(native.arguments) do
		if v.pointer then
			if foundPointer then
				return false
			else
				foundPointer = true
			end
		end
	end

	return native.arguments[#native.arguments].pointer
end

local function printReturnType(type)
	if type.nativeType == 'string' then
		return 'string'
	elseif type.nativeType == 'boolean' then
		return 'bool'
	elseif type.nativeType == 'float' then
		return 'number'
	elseif type.nativeType == 'vector3' then
		return 'number[]'
	elseif type.nativeType == 'int' then
		return 'number'
	elseif type.nativeType == 'object' then
		return 'any'
	else
		return 'number'
	end
end

local function trimAndNormalize(str)
	return trim(str):gsub('/%*', ' -- [['):gsub('%*/', ']] ')
end

local function formatDocString(native)
	local d = parseDocString(native)

	if not d then
		return ''
	end

	local firstLine, nextLines = d.summary:match("([^\n]+)\n?(.*)")

	if not firstLine then
		return ''
	end

	local l = '/**\n * ' .. trimAndNormalize(firstLine) .. "\n"

	for line in nextLines:gmatch("([^\n]+)") do
		l = l .. ' * ' .. trimAndNormalize(line) .. "\n"
	end

	if d.hasParams then
		for _, v in ipairs(d.params) do
			l = l .. ' * @param ' .. v[1] .. ' ' .. v[2] .. '\n'
		end
	end

	if d.returns then
		l = l .. ' * @return ' .. d.returns .. '\n'
	end

	l = l .. ' */\n'

	return l
end

local function printArgument(argument, native)
	local argType = nil
	local retType = nil

	if argument.pointer then
		if argument.type.nativeType == 'int' or argument.type.nativeType == 'float' then
			if isSinglePointerNative(native) then
				argType = 'number'
			else
				retType = 'number'
			end
		elseif argument.type.nativeType == 'vector3' then
			retType = 'number[]'
		else
			retType = 'any /* actually ' .. argument.type.nativeType .. ' */'
		end
	elseif argument.type.name == 'func' then
		argType = 'Function'
	elseif argument.type.name == 'Hash' then
		argType = 'string | number'
	elseif argument.type.name == 'charPtr' then
		argType = 'string'
	elseif argument.type.nativeType == 'int' then
		argType = 'number'
	elseif argument.type.nativeType == 'float' then
		argType = 'number'
	elseif argument.type.nativeType == 'bool' then
		argType = 'boolean'
	end

	local name = argument.name

	-- ts's reserved word
	if langWords[name] then
		name = langWords[name]
  	end

	return name, argType, retType
end

local function formatDefinition(native)
	local argsDefs = {}
	local retTypes = {}

	if native.returns then
		table.insert(retTypes, printReturnType(native.returns))
	end

	if native.arguments then
		for _, argument in pairs(native.arguments) do
			local argumentName, argType, retType = printArgument(argument, native)

			if argType ~= nil then
				table.insert(argsDefs, argumentName .. ': ' .. argType)
			elseif retType ~= nil then
				table.insert(retTypes, retType)
			end
		end
	end

	local retType

	if #retTypes > 1 then
		retType = '[' .. table.concat(retTypes, ', ') .. ']'
	elseif #retTypes == 1 then
		retType = retTypes[1]
	else
		retType = 'void'
	end

	return '(' .. table.concat(argsDefs, ', ') .. '): ' .. retType
end

local function printNative(native)
	local name = printFunctionName(native)
	local doc = formatDocString(native)
	local def = formatDefinition(native)

	local str = string.format("%sdeclare function %s%s;\n", doc, name, def)

	for _, alias in ipairs(native.aliases) do
		str = str .. ("%sdeclare function %s%s;\n"):format(doc, printFunctionName({ name = alias }), def)
	end

	return str
end

for _, v in pairs(_natives) do
	if matchApiSet(v) then
		print(printNative(v))
	end
end
