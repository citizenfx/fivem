-- setup environment for the codegen'd file to execute in
local codeEnvironment = {
	
}

local types = {}
local natives = {}
local curType
local curNative

function codeEnvironment.type(typeName)
	-- create a new type entry
	types[typeName] = {
		name = typeName
	}

	-- set a local
	local typeEntry = types[typeName]

	-- set a closure named after the type in the code environment
	codeEnvironment[typeName] = function(argumentName)
		return {
			type = typeEntry,
			name = argumentName
		}
	end

	codeEnvironment[typeName .. 'Ptr'] = function(argumentName)
		local t = codeEnvironment[typeName](argumentName)

		t.pointer = true

		return t
	end

	-- set the current type
	curType = typeEntry
end

function codeEnvironment.extends(base)
	curType.nativeType = types[base].nativeType
end

function codeEnvironment.nativeType(typeName)
	curType.nativeType = typeName
end

function codeEnvironment.property(propertyName)
	return function(data)

	end
end

function codeEnvironment.method(methodName)
	return function(data)

	end
end

function codeEnvironment.native(nativeName)
	-- create a new entry
	local native = {
		name = nativeName,
		apiset = {}
	}

	table.insert(natives, native)

	-- set the current native to said native
	curNative = native
end

function codeEnvironment.hash(hashString)
	curNative.hash = hashString
end

function codeEnvironment.jhash(hash)
	curNative.jhash = hash
end

function codeEnvironment.arguments(list)
	curNative.arguments = list
end

function codeEnvironment.returns(typeName)
	curNative.returns = types[typeName]
end

function codeEnvironment.apiset(setName)
	table.insert(curNative.apiset, setName)
end

-- load the definition file
function loadDefinition(filename)
	local chunk = loadfile(filename, 't', codeEnvironment)

	chunk()
end

local gApiSet = 'server'

local function matchApiSet(native)
	local apisets = native.apiset

	if #apisets == 0 then
		apisets = { 'client' }
	end

	for _, v in ipairs(apisets) do
		if v == gApiSet or v == 'shared' then
			return true
		end
	end

	return false
end

loadDefinition 'codegen_types.lua'

if #arg > 0 then
	loadDefinition(arg[1])

	gApiSet = 'client'
end

loadDefinition 'codegen_cfx_natives.lua'

local _natives = {}

for _, v in ipairs(natives) do
	table.insert(_natives, v)
end

table.sort(_natives, function(a, b)
	return a.name < b.name
end)

--[[for _, v in pairs(_natives) do
	print(string.format("%s = %s,", v.name, v.hash))
end

os.exit(0)]]

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
print("local _i, _f, _v, _r, _ri, _rf, _s, _rv, _in, _ii, _fi =\n\tCitizen.PointerValueInt(), Citizen.PointerValueFloat(), Citizen.PointerValueVector(),\n\tCitizen.ReturnResultAnyway(), Citizen.ResultAsInteger(), Citizen.ResultAsFloat(), Citizen.ResultAsString(), Citizen.ResultAsVector(),\n\tCitizen.InvokeNative, Citizen.PointerValueIntInitialized, Citizen.PointerValueFloatInitialized\n")

print("local g = _G\n")

print("local function _ch(hash)")
print("\tif g.type(hash) == 'string' then")
print("\t\treturn g.GetHashKey(hash)")
print("\tend\n")
print("\treturn hash")
print("end\n")

print("local Global = setmetatable({}, { __newindex = function(_, n, v)\n\tg[n] = v\nend})\n")

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
	elseif argument.type.name == 'Hash' then
		return '_ch(' .. printArgumentName(argument.name) .. ')'
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
		return '_ri'
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

local function printNative(native)
	local str = string.format("function Global.%s(%s)\n", printFunctionName(native), printArgumentList(native))

	str = str .. string.format("\treturn _in(%s)\n", printInvocationArguments(native))

	str = str .. "end\n"

	return str
end

for _, v in pairs(_natives) do
	if matchApiSet(v) then
		print(printNative(v))
	end
end