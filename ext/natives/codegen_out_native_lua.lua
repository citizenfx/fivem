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

local function isSafeNative(native)
	-- a native is 'safe' for this if
	-- 1. the native has only int/string/bool arguments (float can be a vector, which is bad)
	-- 2. 'trivial' return value (int, float, vector3, string, bool)
	
	local safe = true
	
	if native.name:sub(1, 2) == '0x' then
		safe = false
	end
	
	for argn, arg in pairs(native.arguments) do
		if arg.type.nativeType ~= 'int' and
		   arg.type.nativeType ~= 'bool' and
		   arg.type.nativeType ~= 'string' then
			safe = false
		end
		
		if arg.type.isPointer then
			safe = false
		end
	end
	
	if safe then
		if native.returns then
			if native.returns.nativeType ~= 'int' and
			   native.returns.nativeType ~= 'bool' and
			   native.returns.nativeType ~= 'vector3' and
			   native.returns.nativeType ~= 'float' and
			   native.returns.nativeType ~= 'string' then
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

local function parseArgumentType(type, native)
	local argType

	if type.name == 'Hash' then
		argType = 'uint32_t'
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
	elseif type.name == 'object' then
		argType = 'scrObject'
	end
	
	return argType
end

local function printTypeGetter(argument, native, idx)
	local argType

	if argument.type.name == 'Hash' then
		argType = 'Lua_ToHash(L, ' .. idx .. ')'
	elseif argument.type.name == 'uint' then
		argType = 'lua_utointeger(L, ' .. idx .. ')'
	elseif argument.type.nativeType == 'Any*' then
		argType = 'lua_utointeger(L, ' .. idx .. ')'
	elseif argument.type.nativeType == 'string' then
		argType = 'lua_tostring(L, ' .. idx .. ')'
	elseif argument.type.nativeType == 'int' then
		argType = 'lua_utointeger(L, ' .. idx .. ')'
	elseif argument.type.nativeType == 'float' then
		argType = '(float)lua_utonumber(L, ' .. idx .. ')'
	elseif argument.type.nativeType == 'bool' then
		--argType = 'lua_toboolean(L, ' .. idx .. ')'
		-- 0 is truthy in lua, so let's use raw value access
		argType = '(lua_utointeger(L, ' .. idx .. ') & 0xFF) != 0'
	elseif argument.type.nativeType == 'vector3' then
		argType = 'Lua_ToScrVector(L, ' .. idx .. ')'
	elseif argument.type.name == 'func' then
		argType = 'Lua_ToFuncRef(L, ' .. idx .. ')'
	else
		error('invalid arg in native ' .. native.hash)
	end
	
	return argType
end

local function printTypeSetter(type, native, idx)
	local argType

	if type.name == 'Hash' then
		argType = 'lua_pushinteger(L, ' .. idx .. ')'
	elseif type.name == 'uint' then
		argType = 'lua_pushinteger(L, ' .. idx .. ')'
	elseif type.nativeType == 'Any*' then
		argType = 'lua_pushinteger(L, ' .. idx .. ')'
	elseif type.nativeType == 'string' then
		argType = 'lua_pushstring(L, ' .. idx .. ')'
	elseif type.nativeType == 'int' then
		argType = 'lua_pushinteger(L, ' .. idx .. ')'
	elseif type.nativeType == 'float' then
		argType = 'lua_pushnumber(L, ' .. idx .. ')'
	elseif type.nativeType == 'bool' then
		argType = 'lua_pushboolean(L, ' .. idx .. ')'
	elseif type.nativeType == 'vector3' then
		argType = 'Lua_PushScrVector(L, ' .. idx .. ')'
	elseif type.name == 'object' then
		argType = 'Lua_PushScrObject(L, ' .. idx .. ')'
	end
	
	return argType
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
	local expectedArgs = #native.arguments

	for argn, arg in pairs(native.arguments) do
		local ptr = arg.pointer
		local isInitialized = arg.initializedPointer
		local type = parseArgumentType(arg.type, native)
		local name = arg.name
	
		if ptr then
			refValNum = refValNum + 1
			local refName = 'ref_' .. name

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

	for argn, arg in pairs(native.arguments) do
		if arg.pointer then
			n = n .. t .. ("nCtx.SetArgument(%d, &ref_%s);\n"):format(aIdx, arg.name)
			
			if arg.initializedPtr then
				lIdx = lIdx + 1
			end
			
			aIdx = aIdx + 1
		else
			n = n .. t .. ("nCtx.SetArgument(%d, lua_asserttop(L, %d) ? %s : 0); // %s\n"):format(aIdx, aIdx + 1, printTypeGetter(arg, native, lIdx), arg.name)
			
			lIdx = lIdx + 1
			
			aIdx = aIdx + ((arg.type.nativeType == 'vector3') and 3 or 1)
		end
	end
	
	n = n .. t .. ("LUA_EXC_WRAP_START(0x%016x)\n"):format(native.hash)
	n = n .. t .. ("nCtx.Invoke(L, 0x%016x);\n"):format(native.hash)
	n = n .. t .. ("LUA_EXC_WRAP_END(0x%016x)\n"):format(native.hash)
	
	local retValNum = refValNum
	
	if native.returns then
		local type = parseArgumentType(native.returns, native)
	
		n = n .. t .. ("%s retval = nCtx.GetResult<%s>();\n"):format(type, type)
		
		n = n .. t .. ("%s;\n"):format(printTypeSetter(native.returns, native, 'retval'))
		
		retValNum = retValNum + 1
	end
	
	for argn, arg in pairs(native.arguments) do
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