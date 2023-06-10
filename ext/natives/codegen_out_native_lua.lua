-- natives requiring initialized pointers
local unsupList = {
	'DELETE_ENTITY',
	'DELETE_OBJECT',
	'DELETE_PED',
	'DELETE_VEHICLE',
	'DELETE_MISSION_TRAIN',
	'SET_ENTITY_AS_NO_LONGER_NEEDED',
	'SET_OBJECT_AS_NO_LONGER_NEEDED',
	'SET_PED_AS_NO_LONGER_NEEDED',
	'SET_VEHICLE_AS_NO_LONGER_NEEDED',
	'SET_MISSION_TRAIN_AS_NO_LONGER_NEEDED'
}

local unsup = {}

for _, v in ipairs(unsupList) do
	unsup[v] = true
end

--[[ from codegen_out_lua.lua --]]
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

	if #native.arguments > 0 then
		return native.arguments[#native.arguments].pointer
	end
	return false
end

--[[ Safe parameter types for fxv2 native invocation --]]
local safeArguments = {
	int = true,
	float = true,
	bool = true,
	string = true,
	Hash = true,
}

--[[ Safe native return types for fxv2 native invocation --]]
local safeResults = {
	int = true,
	float = true,
	bool = true,
	string = true,
	Hash = true,
	vector3 = true,
	object = true,
}

--[[
	A native is 'safe' if:
		1. Has a known or partially known name.
		2. Has only int/float/string/bool/Hash arguments.
		3. Has a 'trivial' return type (void, int, float, vector3, string, bool).

	Known compatibility changes:
		1. Native handler will not implicitly unroll vectors arguments.	
		2. In the old native handler, natives with boolean 'out' pointers will 
			convert the boolean type to an int, e.g., GetShapeTestResult: 
			"_i --\[\[ actually bool \]\]". This value will now remain boolean.
--]]
local function isSafeNative(native)
	if native.name:sub(1, 2) == '0x' then
		return false
	end

	local safe = true
	local singlePointer = isSinglePointerNative(native)
	for argn=1,#native.arguments do
		local arg = native.arguments[argn]
		local nativeType = arg.type.nativeType or 'Any'

		if arg.pointer then
			if singlePointer then
				safe = false
			elseif nativeType ~= "vector3" and not safeArguments[nativeType] then
				safe = false
			end
		elseif not safeArguments[nativeType] then
			safe = false
		end
	end

	if safe then
		if native.returns then
			if not safeResults[native.returns.nativeType or 'void'] then
				safe = false
			end
		end
	end

	return safe
end

local function isUnsupportedNative(native)
	return unsup[native.name] and true or false
end

local function printFunctionName(native)
	return native.name:lower():gsub('_([%a%d])', string.upper):gsub('0x', 'N_0x'):gsub('(%a)(.+)', function(a, b)
		return a:upper() .. b
	end)
end

local function parseArgumentType(type, native, return_type)
	local argType

	if type.name == 'Hash' then
		argType = 'int32_t'
	elseif type.name == 'uint' then
		argType = 'uint32_t'
	elseif type.nativeType == 'Any*' then
		argType = 'uint64_t'
	elseif type.nativeType == 'string' then
		argType = 'const char*'
	elseif type.nativeType == 'int' then
		if type.subType == 'long' then
			argType = 'int64_t'
		else
			argType = 'int32_t'
		end
	elseif type.nativeType == 'float' then
		argType = 'float'
	elseif type.nativeType == 'bool' then
		argType = 'bool'
	elseif type.nativeType == 'vector3' then
		argType = 'scrVectorLua'
		if return_type then
			argType = "const " .. argType
		end
	elseif type.name == 'object' then
		argType = 'scrObject'
		if return_type then
			argType = "const " .. argType
		end		
	end
	
	return argType
end

local function printTypeGetter(argument, native, idx)
	local template = "LuaArgumentParser::ParseArgument<%s>(L, " .. idx .. ")"

	local argType
	if argument.type.name == 'Hash' then
		argType = template:format("uint32_t")
	elseif argument.type.name == 'uint' then
		argType = template:format("lua_Integer")
	elseif argument.type.nativeType == 'Any*' then
		argType = template:format("lua_Integer")
	elseif argument.type.nativeType == 'string' then
		argType = template:format("const char*")
	elseif argument.type.nativeType == 'int' then
		argType = template:format("lua_Integer")
	elseif argument.type.nativeType == 'float' then
		argType = template:format("float")
	elseif argument.type.nativeType == 'bool' then
		argType = template:format("bool")
	elseif argument.type.nativeType == 'vector3' then
		argType = template:format("scrVectorLua")
	elseif argument.type.name == 'func' then
		argType = "LuaArgumentParser::ParseFunctionReference(L, " .. idx .. ")"
	elseif argument.type.name == 'object' then
		-- EXPERIMENTAL: Requires testing and safeArguments update.
		argType = "LuaArgumentParser::ParseObject(L, " .. idx .. ")"
	else
		error('invalid arg in native ' .. native.hash)
	end
	
	return argType
end

local function printTypeSetter(type, native, retval)
	local template = "LuaArgumentParser::PushObject<%s>(L, %s)"

	local argType
	if type.name == 'Hash' then
		argType = template:format("int32_t", retval)
	elseif type.name == 'uint' then
		argType = template:format("uint32_t", retval)
	elseif type.nativeType == 'Any*' then
		argType = template:format("lua_Integer", ("(lua_Integer)%s"):format(retval))
	elseif type.nativeType == 'string' then
		argType = template:format("const char*", retval)
	elseif type.nativeType == 'int' then
		if type.subType == 'long' then
			argType = template:format("int64_t", retval)
		else
			argType = template:format("int32_t", retval)
		end
	elseif type.nativeType == 'float' then
		argType = template:format("float", retval)
	elseif type.nativeType == 'bool' then
		argType = template:format("bool", retval)
	elseif type.nativeType == 'vector3' then
		argType = template:format("const scrVectorLua&", retval)
	elseif type.name == 'object' then
		argType = template:format("const scrObject&", retval)
	end
	
	return argType
end

--[[
	Return true if the native parameters starting at 'arg' may correspond to a 
	floating point sequence that can be unrolled from a LUA_TVECTOR type.

	All triples are extracted from the V natives repository.
--]]
local function isVectorSequence(native, arg)
	local fields = { 
		{ 'x', 'y', 'z' },
		{ 'x1', 'y1', 'z1' },
		{ 'x2', 'y2', 'z2' },
		{ 'posX', 'posY', 'posZ' },
		{ 'dirX', 'dirY', 'dirZ' },
		{ 'rotX', 'rotY', 'rotZ' },
		{ 'roll', 'pitch', 'yaw' },
		{ 'coordsX', 'coordsY', 'coordsZ' },
		{ 'offsetX', 'offsetY', 'offsetZ' },
		{ 'rotationX', 'rotationY', 'rotationZ' },
		{ 'hookOffsetX', 'hookOffsetY', 'hookOffsetZ' },
		{ 'destinationX', 'destinationY', 'destinationZ' },
		{ 'goToLocationX', 'goToLocationY', 'goToLocationZ' },
		{ 'focusLocationX', 'focusLocationY', 'focusLocationZ' },
	}

	for i=1,#fields do
		local result = true

		local field = fields[i]
		for j=1,#field do
			local arg = native.arguments[arg + j - 1]
			if arg == nil or arg.type.name ~= "float" or arg.name ~= field[j] then
				result = false
			end
		end

		if result then
			return result
		end     
	end

	return false
end

local function printNative(native)
	local nativeName = printFunctionName(native)
	local n = ''
	
	if not native.arguments then
		native.arguments = {}
	end
	
	n = n .. ("int Lua_Native_%s(lua_State* L) // %s\n"):format(native.hash, nativeName)
	n = n .. "{\n"
	
	local t = "\t"
	
	local refValNum = 0
	local refHasVector = false
	local expectedArgs = #native.arguments

	for argn=1,#native.arguments do
		local arg = native.arguments[argn]
		local ptr = arg.pointer
		local isInitialized = arg.initializedPointer
		local type = parseArgumentType(arg.type, native)
		local name = arg.name
	
		if ptr then
			refValNum = refValNum + 1
			local refName = 'ref_' .. name
			if arg.type.nativeType == 'vector3' then
				refHasVector = true
			end

			n = n .. t .. type .. ' ' .. refName
			
			if isInitialized then
				n = n .. ' = ' .. printTypeGetter(arg, native, argn)
			else
				expectedArgs = expectedArgs - 1
			end
			
			n = n .. ';\n'
		end
	end
	
	if refValNum > 1 then
		n = n .. '\n'
	end
	
	n = n .. t .. ("static LuaNativeWrapper nW(0x%016x);\n"):format(native.hash)
	n = n .. t .. ("LuaNativeContext nCtx(&nW, %d);\n"):format(#native.arguments)
	
	--n = n .. t .. ("ASSERT_LUA_ARGS(%d);\n"):format(expectedArgs)
	
	local lIdx = 1
	local aIdx = 0
	
	-- n = n .. t .. ("const int Ltop = lua_gettop(L);\n")

	for argn=1,#native.arguments do
		local arg = native.arguments[argn]
		if arg.pointer then
			n = n .. t .. ("nCtx.SetArgument(%d, &ref_%s);\n"):format(aIdx, arg.name)
			
			if arg.initializedPtr then
				lIdx = lIdx + 1
			end
			
			aIdx = aIdx + 1
		else
			n = n .. t .. ("nCtx.SetArgument(%d, %s); // %s\n"):format(aIdx, printTypeGetter(arg, native, lIdx), arg.name)
			
			lIdx = lIdx + 1
			
			aIdx = aIdx + ((arg.type.nativeType == 'vector3') and 3 or 1)
		end
	end
	
	n = n .. t .. ("LUA_EXC_WRAP_START(0x%016x)\n"):format(native.hash)
	n = n .. t .. ("nCtx.Invoke(L, 0x%016x);\n"):format(native.hash)
	if refHasVector then
		n = n .. t .. "#if !defined(IS_FXSERVER)\n"
		n = n .. t .. "nCtx.rawCxt.SetVectorResults();\n"
		n = n .. t .. "#endif\n"
	end
	n = n .. t .. ("LUA_EXC_WRAP_END(0x%016x)\n"):format(native.hash)
	
	local retValNum = refValNum
	
	if native.returns then
		local type = parseArgumentType(native.returns, native, true)
	
		n = n .. t .. ("%s retval = nCtx.GetResult<%s>();\n"):format(type, type)
		
		n = n .. t .. ("%s;\n"):format(printTypeSetter(native.returns, native, 'retval'))
		
		retValNum = retValNum + 1
	end
	
	for argn=1,#native.arguments do
		local arg = native.arguments[argn]
		local ptr = arg.pointer
		local isInitialized = arg.initializedPointer
		local type = parseArgumentType(arg.type, native)
		local name = arg.name
	
		if ptr then
			local refName = 'ref_' .. name

			n = n .. t .. ("%s;\n"):format(printTypeSetter(arg.type, native, refName))
		end
	end
	
	n = n .. t .. ("return %d;\n"):format(retValNum)
	
	n = n .. "}\n"
	
	return n
end

local function printNativeRef(native)
	if isUnsupportedNative(native) then
		return ''
	end

	local nativeName = printFunctionName(native)
	local n = ''
	
	n = n .. ("\t{ \"%s\", Lua_Native_%s },\n"):format(nativeName, native.hash)
	
	for _, alias in ipairs(native.aliases) do
		local aliasName = printFunctionName({ name = alias })
		
		n = n .. ("\t{ \"%s\", Lua_Native_%s },\n"):format(aliasName, native.hash)
	end
	
	return n
end

local safeNatives = {}

for _, v in pairs(_natives) do
	if matchApiSet(v) and isSafeNative(v) then
		safeNatives[v.hash] = true
		print(printNative(v))
	end
end

print("static const Lua_NativeMap natives = {")

for _, v in pairs(_natives) do
	if safeNatives[v.hash] and matchApiSet(v) then
		print(printNativeRef(v))
	end
end
	
print("};")