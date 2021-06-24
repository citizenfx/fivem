local USE_SPLIT_LUA, USE_DOC_LUA = ...
local USE_SPLIT_LUA_DIRECT = USE_SPLIT_LUA and gApiSet ~= 'server'

if USE_SPLIT_LUA or USE_DOC_LUA then
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

print("local function _obj(obj)")
print("\tlocal s = msgpack.pack(obj)")
print("\treturn s, #s")
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

if not USE_DOC_LUA then
	print("_ENV = nil\n")
end

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
	elseif argument.type.name == 'object' then
		return '_obj(' .. printArgumentName(argument.name) .. ')'
	elseif argument.type.name == 'func' then
		return '_mfr(' .. printArgumentName(argument.name) .. ')'
	elseif argument.type.name == 'Hash' then
		return '_ch(' .. printArgumentName(argument.name) .. ')'
	elseif argument.type.nativeType == 'string' then
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
		USE_SPLIT_LUA_DIRECT and 'fn' or native.hash
	}

	if native.arguments then
		for _, v in pairs(native.arguments) do
			table.insert(args, printArgument(v, native))
		end
	end

	if native.returns then
		if native.returns.nativeType ~= 'bool' then
			table.insert(args, printReturnType(native.returns))
		else
			table.insert(args, '_r')
		end
	end

	return table.concat(args, ', ')
end

local function formatCommentedLine(line, indent)
	local indentStr = string.rep('\t', indent or 0)
	return line:gsub('\r\n', '\n'):gsub('\n', '\n-- ' .. indentStr)
end

local function printLuaType(type)
	if type.nativeType == 'float' then
		return 'number'
	elseif type.nativeType == 'int' or type.nativeType == 'long' then
		return 'integer'
	elseif type.nativeType == 'bool' then
		return 'boolean'
	end

	return type.nativeType
end

local function formatDocString(native)
	local d = parseDocString(native)

	if not d then
		if USE_DOC_LUA then
			d = {
				summary = ''
			}
		else
			return ''
		end
	end

	local firstLine, nextLines = d.summary:match("([^\n]+)\n?(.*)")

	if not firstLine then
		if not USE_DOC_LUA then
			return ''
		else
			firstLine = ''
			nextLines = ''
		end
	end

	local l = '--- ' .. trim(firstLine) .. "\n"
	
	-- doclua wants `---` stuff
	local docPrefix = USE_DOC_LUA and '--- ' or '-- '

	for line in nextLines:gmatch("([^\n]+)") do
		l = l .. docPrefix .. trim(line) .. "\n"
	end

	if USE_DOC_LUA then
		-- params
		local paramToDesc = {}
		
		if d.hasParams then
			for _, v in ipairs(d.params) do
				paramToDesc[printArgumentName(v[1])] = v[2]
			end
		end
	
		if native.arguments then
			for _, v in ipairs(native.arguments) do
				if not v.pointer or isSinglePointerNative(native) then
					local an = printArgumentName(v.name)
					l = l .. docPrefix .. '@param ' .. an .. ' ' .. printLuaType(v.type) .. ' ' .. (paramToDesc[an] and ('@comment ' .. paramToDesc[an]) or '') .. '\n'
				end
			end
		end
		
		if native.returns then
			if d.returns then
				l = l .. docPrefix .. '@return ' .. printLuaType(native.returns) .. ' @comment ' .. formatCommentedLine(d.returns, 2) .. '\n'
			else
				l = l .. docPrefix .. '@return ' .. printLuaType(native.returns) .. '\n'
			end
		end
		
		if native.arguments then
			for _, v in ipairs(native.arguments) do
				if v.pointer then
					local an = printArgumentName(v.name)
					l = l .. docPrefix .. '@return ' .. printLuaType(v.type) .. ' ' .. an --[[.. (paramToDesc[an] or '')]] .. '\n'
				end
			end
		end
	else
		if d.hasParams then
			for _, v in ipairs(d.params) do
				l = l .. docPrefix .. '@param ' .. v[1] .. ' ' .. v[2] .. '\n'
			end
		end

		if d.returns then
			l = l .. docPrefix .. '@return ' .. formatCommentedLine(d.returns, 2) .. '\n'
		end
	end

	return l
end

local idx = 0

local function printNative(native)
	local str = ""

	local function printThis(name)
		local prefix = ""
		
		if USE_SPLIT_LUA_DIRECT then
			prefix = string.format("local fn = _gn(%s)\n", native.hash)
		end
	
		local str = string.format("%s%sfunction %s%s(%s)\n", prefix, formatDocString(native), USE_DOC_LUA and '' or 'Global.', name, printArgumentList(native))

		if not USE_DOC_LUA then
			local preCall = ''
			local postCall = ''
		
			if native.returns and native.returns.nativeType == 'object' then
				preCall = 'msgpack.unpack('
				postCall = ')'
			end
		
			str = str .. string.format("\treturn %s_in%s(%s)%s\n", preCall, USE_SPLIT_LUA_DIRECT and '2' or '', printInvocationArguments(native), postCall)
		end
	
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
	elseif USE_DOC_LUA then
		local function saveThis(name, str)
			local f = io.open(('%s/natives_%d.lua'):format(os.getenv('NATIVES_DIR'), math.floor(idx / 100)), 'a')
			f:write(str)
			f:close()
		end
		
		saveThis(printFunctionName(native), printThis(printFunctionName(native)))

		for _, alias in ipairs(native.aliases) do
			saveThis(printFunctionName({ name = alias }), printThis(printFunctionName({ name = alias })))
		end
		
		idx = idx + 1
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
		if USE_SPLIT_LUA or USE_DOC_LUA then
			printNative(v)
		else
			print(printNative(v))
		end
	end
end
