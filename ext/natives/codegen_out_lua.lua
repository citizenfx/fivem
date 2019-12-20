local USE_SPLIT_LUA = ...

if USE_SPLIT_LUA then
	if not os.getenv('NATIVES_DIR') or #os.getenv('NATIVES_DIR') < 2 then
		error('no dir')
	end
end

local cfx = require('cfx')

local function printFunctionName(native)
	return native.name:lower():gsub('0x', 'n_0x'):gsub('_(%a)', string.upper):gsub('(%a)(.+)', function(a, b)
		return a:upper() .. b
	end)
end

-- sort the natives table
local _natives = {}

for _, v in ipairs(natives) do
	table.insert(_natives, v)
end

table.sort(_natives, function(a, b)
	return printFunctionName(a) < printFunctionName(b)
end)

-- output the Lua low-level native definition file

-- header bit
print("local _i, _f, _v, _r, _ri, _rf, _rl, _s, _rv, _ro, _in, _ii, _fi =\n\tCitizen.PointerValueInt(), Citizen.PointerValueFloat(), Citizen.PointerValueVector(),\n\tCitizen.ReturnResultAnyway(), Citizen.ResultAsInteger(), Citizen.ResultAsFloat(), Citizen.ResultAsLong(), Citizen.ResultAsString(), Citizen.ResultAsVector(), Citizen.ResultAsObject(),\n\tCitizen.InvokeNative, Citizen.PointerValueIntInitialized, Citizen.PointerValueFloatInitialized\n")

print("local g = _G")
print("local rs = rawset")
print("local msgpack = msgpack")

print("local _tostring = tostring")
print("local function _ts(num)")
print("\tif num == 0 or not num then -- workaround for users calling string parameters with '0', also nil being translated")
print("\t\treturn nil")
print("\tend")
print("\treturn _tostring(num)")
print("end")

print("local function _ch(hash)")
print("\tif g.type(hash) == 'string' then")
print("\t\treturn g.GetHashKey(hash)")
print("\tend\n")
print("\treturn hash")
print("end\n")

print("local function _mfr(fn)")
print("\treturn g.Citizen.GetFunctionReference(fn)")
print("end\n")

print("local Global = setmetatable({}, { __newindex = function(_, n, v)\n\tg[n] = v\n\n\trs(_, n, v)\nend})\n")

print("_ENV = nil\n")

-- functions
local function printArgumentName(name)
	if name == 'end' then
		return 'end_'
	elseif name == 'repeat' then
		return 'repeat_'
	end

	return name
end

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

local function printArgument(argument, native)
	if argument.pointer then
		if argument.type.nativeType == 'int' then
			if isSinglePointerNative(native) then
				return '_ii(' .. printArgumentName(argument.name) .. ') --[[ may be optional ]]'
			else
				return '_i'
			end
		elseif argument.type.nativeType == 'float' then
			if isSinglePointerNative(native) then
				return '_fi(' .. printArgumentName(argument.name) .. ') --[[ may be optional ]]'
			else
				return '_f'
			end
		elseif argument.type.nativeType == 'vector3' then
			return '_v'
		else
			return '_i --[[ actually ' .. argument.type.nativeType .. ' ]]'
		end
	elseif argument.type.name == 'func' then
		return '_mfr(' .. printArgumentName(argument.name) .. ')'
	elseif argument.type.name == 'Hash' then
		return '_ch(' .. printArgumentName(argument.name) .. ')'
	elseif argument.type.name == 'charPtr' then
		return '_ts(' .. printArgumentName(argument.name) .. ')'
	end

	return printArgumentName(argument.name)
end

local function printArgumentList(native)
	if not native.arguments then
		return ''
	end

	local args = {}

	for _, v in ipairs(native.arguments) do
		if not v.pointer or isSinglePointerNative(native) then
			table.insert(args, printArgumentName(v.name))
		end
	end

	return table.concat(args, ', ')
end

local function printReturnType(type)
	if type.nativeType == 'string' then
		return '_s'
	elseif type.nativeType == 'float' then
		return '_rf'
	elseif type.nativeType == 'vector3' then
		return '_rv'
	elseif type.nativeType == 'int' then
		if type.subType == 'long' then
			return '_rl'
		else
			return '_ri'
		end
	elseif type.nativeType == 'Any*' then
		return '_ri'
	elseif type.nativeType == 'object' then
		return '_ro'
	end
end

local function printInvocationArguments(native)
	local args = {
		native.hash
	}

	if native.arguments then
		for _, v in pairs(native.arguments) do
			table.insert(args, printArgument(v, native))
		end
	end

	if native.returns then
		table.insert(args, '_r')

		if native.returns.nativeType ~= 'bool' then
			table.insert(args, printReturnType(native.returns))
		end
	end

	return table.concat(args, ', ')
end

local function formatCommentedLine(line, indent)
	local indentStr = string.rep('\t', indent or 0)
	return line:gsub('\r\n', '\n'):gsub('\n', '\n-- ' .. indentStr)
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

	local l = '--- ' .. trim(firstLine) .. "\n"

	for line in nextLines:gmatch("([^\n]+)") do
		l = l .. '-- ' .. trim(line) .. "\n"
	end

	if d.hasParams then
		for _, v in ipairs(d.params) do
			l = l .. '-- @param ' .. v[1] .. ' ' .. v[2] .. '\n'
		end
	end

	if d.returns then
		l = l .. '-- @return ' .. formatCommentedLine(d.returns, 2) .. '\n'
	end

	return l
end

local function printNative(native)
	local str = ""

	local function printThis(name)
		local str = string.format("%sfunction Global.%s(%s)\n", formatDocString(native), name, printArgumentList(native))

		local preCall = ''
		local postCall = ''
	
		if native.returns and native.returns.nativeType == 'object' then
			preCall = 'msgpack.unpack('
			postCall = ')'
		end
	
		str = str .. string.format("\treturn %s_in(%s)%s\n", preCall, printInvocationArguments(native), postCall)
	
		str = str .. "end\n"

		return str
	end

	if USE_SPLIT_LUA then
		local function saveThis(name, str)
			local f = io.open(('%s/0x%08x.lua'):format(os.getenv('NATIVES_DIR'), cfx.hash(name)), 'w')
			f:write(str)
			f:close()
		end

		saveThis(printFunctionName(native), printThis(printFunctionName(native)))

		for _, alias in ipairs(native.aliases) do
			saveThis(printFunctionName({ name = alias }), printThis(printFunctionName({ name = alias })))
		end
	else
		str = str .. printThis(printFunctionName(native))

		for _, alias in ipairs(native.aliases) do
			str = str .. ("Global.%s = Global.%s\n"):format(printFunctionName({ name = alias }), printFunctionName(native))
		end
	end

	return str
end

for _, v in pairs(_natives) do
	if matchApiSet(v) then
		if USE_SPLIT_LUA then
			printNative(v)
		else
			print(printNative(v))
		end
	end
end
