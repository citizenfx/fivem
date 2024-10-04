-- Products
local PRODUCTS = {
	['server'] = { 'IS_FXSERVER', 'Server' },
	['global'] = { 'GTA_FIVE',    'FiveM' },
	['rdr3'] =   { 'IS_RDR3',     'RedM' },
	['ny'] =     { 'GTA_NY',      'LibertyM' },
}

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

local t1, t2, t3, t4 = '\t', '\t\t', '\t\t\t', '\t\t\t\t'
compatibility = {}    -- need to be global
version = { 2, 0, 0 } -- need to be global
	
-- default code patterns, changing these can result in bad code generation or type mismatch
local valType = 'ulong'	-- used as the default

local valBase = { 'ulong {0}', '{0}' }
local valWrap = { nil, 'N64.Val({0})', nil }

local fixed = 'fixed(void* p_{0} = &{0})' -- used to fix parameters and get their pointer
local refBase = { 'ref ulong {0}', '(ulong)p_{0}', fixed }
local refWrap = { 'var _{0} = N64.Val({0})', 'ref _{0}', '{0} = N64.To_{1}(_{0})' }

-- Explanation:
-- .            = takes any character, used for custom code generation
-- %W           = any valid character for types and/or methods
-- {0} or {0-1} = input that'll be replaced, '{0}' will be replaced with argument names, '{1}' will be replaced by type names
-- 
-- [requested_type] = {
--     [1] = [%W]                                      -- wrapper argument and return type (unless overriden)
--     [2] = -- base method code (invocation)
--     {
--         [1] = [.{0}],       [required]              -- parameter declaration
--         [2] = [.{0}],       [required]              -- how will the parameter be passed into the ulong[] buffer, e.g.: for strings we use "N64.Str({0})"
--         [3] = [.{0}],                               -- pre stack code used to pin/fix memory or a using statement
--     },
--     [3] = -- wrapper method code
--     {
--         [1] = [.{0}],                               -- pre base-call code, e.g.: "var _{0} = N64.Val({0})" so we can pass {0} as a ref
--         [2] = [.{0}],       [required]              -- how will the parameter be passed into the base function, e.g.: "ref _{0}" if using the above pre base-call code
--         [3] = [.{0}]                                -- post base-call code, e.g.: "{0} = _{0}" to store the retrieved value back into the given parameter
--     },
--     [4] = -- return overrides, can be used to dereference types, send pointers back as ulong, etc.
--     {
--         [1] = [%W],         or "ulong"              -- base method return type, e.g.: "int" by dereferencing the "ref int"
--         [2] = [.{0-1]}],    or "*({1}*){0}"         -- how the base function will interpret the ulong* value(s)
--         [3] = [%W],         or "ulong"              -- wrapper method return type, e.g.: "object" by deserializing the "OutPacket"
--         [4] = [%W{0-1}]     or "N64.To_{1}({0})"    -- how wrapper functions will interpret the base function's return value
--     },
--     typeSize = [0-9]        or 1                    -- override the amount of 64 bit slots taken for this type
--     compatType = [%W]       or wrapperType[4][1]    -- use this type for compat type matching instead of the parameter or return types, reduces the amount of types
--	}

local wrapperTypes = {
	[false] = -- by value
	{		
		['bool'] =    { 'bool', valBase, valWrap },
		['byte'] =    { 'byte', valBase, valWrap },
		['ushort'] =  { 'ushort', valBase, valWrap },
		['uint'] =    { 'uint', valBase, valWrap },
		['ulong'] =   { 'ulong', { 'ulong {0}', '{0}' }, { nil, '{0}', nil } },
		['sbyte'] =   { 'sbyte', valBase, valWrap },
		['short'] =   { 'short', valBase, valWrap },
		['int'] =     { 'int', valBase, valWrap },
		['long'] =    { 'long', valBase, valWrap },
		['float'] =   { 'float', valBase, valWrap },
		['double'] =  { 'double', valBase, valWrap },
		
		['Hash'] =    { 'uint', valBase, valWrap },
		['func'] =    { 'System.Delegate',
			{ 'InFunc {0}', '(ulong)p_{0}', 'fixed(void* p_{0} = {0}.value)' },
			{ nil, '{0}', nil },
			{ 'OutFunc', '*(OutFunc*){0}', 'System.Delegate', '(System.Delegate){0}' }},
		['string'] =  { 'CString',
			{ 'CString {0}', '(ulong)p_{0}', 'fixed(void* p_{0} = {0}?.value)' },
			{ nil, '{0}', nil },
			{ 'OutString', '*(OutString*){0}', 'OutString', '{0}' }, compatType = 'string' },
		['object'] =  { 'object',
			{ 'InPacket {0}', '(ulong)p_{0}, (ulong){0}.value?.LongLength', 'fixed(void* p_{0} = {0}.value)' },
			{ nil, 'InPacket.Serialize({0})', nil },
			{ 'OutPacket', '*(OutPacket*){0}', 'object', '{0}.Deserialize()' }, typeSize = 2, compatType = 'object' },
		['Vector3'] = { 'Vector3',
			{ 'ulong {0}_x, ulong {0}_y, ulong {0}_z', '{0}_x, {0}_y, {0}_z' },
			{ nil, 'N64.Val({0}.X), N64.Val({0}.Y), N64.Val({0}.Z)', nil },
			{ 'Vector3', '(Vector3)(*(NativeVector3*){0})', 'Vector3', '{0}' }, typeSize = 3
		},
	},
	[true] = -- by ref
	{
		['bool'] =    { 'ref bool', refBase, refWrap, { 'bool' } },
		['byte'] =    { 'ref byte', refBase, refWrap, { 'byte' } },
		['ushort'] =  { 'ref ushort', refBase, refWrap, { 'ushort' } },
		['uint'] =    { 'ref uint', refBase, refWrap, { 'uint' } },
		['ulong'] =   { 'ref ulong', refBase, { nil, 'ref {0}', nil }, { 'ulong' }},
		['sbyte'] =   { 'ref sbyte', refBase, refWrap, { 'sbyte' } },
		['short'] =   { 'ref short', refBase, refWrap, { 'short' } },
		['int'] =     { 'ref int', refBase, refWrap, { 'int' } },
		['long'] =    { 'ref long', refBase, refWrap, { 'long' } },
		['float'] =   { 'ref float', refBase, refWrap, { 'float' } },
		['double'] =  { 'ref double', refBase, refWrap, { 'double' } },
		
		['Hash'] =    { 'ref uint', refBase, refWrap, { 'uint', '*(uint*){0}', '{0}' }, { 'uint' } },
		['Vector3'] = { 'ref Vector3', { 'ref Vector3 {0}', '(ulong)p_{0}', fixed }, { nil, 'ref {0}', nil }, { 'Vector3', nil, 'Vector3' } },
	}
}

-- some types have different nativeType values than what we're using in C#, correct them here
-- can be seen as: overrideParamTypes[isPointer][type.name] or type.nativeType
local overrideParamTypes = {
	[false] = -- by value
	{
		['Hash'] = { 'uint', false },
		['Any'] = { 'long', false },
		['Vector3'] = { 'Vector3', false },
		['long'] = { 'long', false },
		['ulong'] = { 'ulong', false },
	},
	[true] = -- by ref
	{
		['Hash'] = { 'uint', true },
		['Any'] = { 'long', true },
		['AnyPtr'] = { 'long', true },
		['Vector3'] = { 'Vector3', true },
	}
}

local overrideReturnTypes = {
	['Hash'] = 'uint',
	['Any'] = 'long',
	['Any*'] = 'long',
	['AnyPtr'] = 'long',
	['Vector3'] = 'Vector3',
	
	['ulong*'] = 'ulong', -- compat file uses ulong* instead of Any*
	['long'] = 'long',
	['ulong'] = 'ulong',
}

-- Explanation:
--
-- compatWrapperTypes[to_type][from_type]
-- 
-- {
---    [1] = [.{0}],                   -- pre base-call code, e.g.: "var _{0} = N64.Val({0})" so we can pass {0} as a ref
--     [2] = [.{0}],       [required]  -- how will the parameter be passed into the base function, e.g.: "ref _{0}" if using the above pre base-call code
--     [3] = [.{0}]                    -- post base-call code, e.g.: "{0} = _{0}" to store the retrieved value back into the given parameter
--     [4] = [.{0}]        or '{0}'    -- return value conversion
-- }
-- 
-- [*]['default'] are used for the missing arguments to the base method
-- 
-- SEGFAULT : (chance of a) Read/Write Access Violation

local compatWrapperTypes = {
	['string'] = {
		['string'] =    { nil, '{0}', nil },
		['ulong'] =     { nil, 'null', nil, 'default' },
		['ulong*'] =    { nil, 'null', nil, 'null' }, -- prev: SEGFAULT
		['Vector3'] =   { nil, 'null', nil, 'default' }, -- shouldn't appear
		['Vector3*'] =  { nil, 'null', nil, 'null' },
		['default'] =   { nil, 'null', nil, 'default' },
	},
	['ulong'] = {
		['string'] =   { nil, '0', nil, '0' },
		['ulong'] =    { nil, '{0}', nil },
		['ulong*'] =   { nil, '{0}', nil },
		['Vector3'] =  { nil, 'N64.Val({0}.X)', nil, 'N64.Val({0}.X)' }, -- shouldn't appear
		['Vector3*'] = { nil, 'N64.Val({0}.X)', nil, 'N64.Val({0}.X)' },
		['default'] =  { nil, '0', nil },
	},
	['ulong*'] = {
		['string'] =   { 'ulong _{0}', 'ref _{0}', nil }, -- prev: SEGFAULT
		['ulong'] =    { nil, 'ref {0}', nil }, -- prev: SEGFAULT
		['ulong*'] =   { nil, 'ref {0}', nil },
		['Vector3'] =  { 'ulong _{0} = N64.Val({0}.X)', 'ref _{0}', nil },
		['Vector3*'] = { 'ulong _{0} = N64.Val({0}.X)', 'ref _{0}', '{0}.X = N64.To_float(_{0})' },
		['default'] =  { 'ulong _{0} = 0', 'ref _{0}', nil },
	},
	['Vector3'] = {
		['string'] =   { nil, 'new Vector3(0.0f)', nil, 'default' },
		['ulong'] =    { nil, 'new Vector3(N64.To_float({0}))', nil, 'new Vector3(N64.To_float({0}))' },
		['ulong*'] =   { nil, 'new Vector3({0})', nil, 'new Vector3(N64.To_float({0}))' }, -- ptr interpreted as the Vec3's float X, ref var doesn't change
		['Vector3'] =  { nil, '{0}', nil },
		['Vector3*'] = { nil, '{0}', nil },
		['default'] =  { nil, 'default(Vector3)', nil },
	},
	['Vector3*'] = {
		['string'] =   { 'Vector3 _{0};\n', 'ref _{0}', nil, '#error unsupported pointer conversion from string to non-string type' },
		['ulong'] =    { 'Vector3 _{0} = new Vector3(N64.To_float({0}))', 'ref _{0}', nil },
		['ulong*'] =   { 'Vector3 _{0} = new Vector3({0})', 'ref _{0}', '{0} = N64.Val(_{0}.X)', '#error unsupported pointer conversion to a smaller type' },
		['Vector3'] =  { nil, 'ref {0}', nil },
		['Vector3*'] = { nil, 'ref {0}', nil },
		['default'] =  { 'Vector3 _{0} = default', 'ref _{0}', nil },
	},
}

local function GetOrDefault(tbl, key, def)
	return (tbl and tbl[key]) or def
end

local function ToCType(typ)
	-- e.g.: turn 'ref uint' into `uint*`
    local s1, s2 = typ:match('^(%w+)%s?(%w*)')
    return s2 ~= '' and s2..'*' or s1
end

local function ArraysEqual(left, right)
	local length = #left;	
	if (length ~= #right) then return false; end
	
	for i = 1, length do
		if (left[i] ~= right[i]) then return false; end
	end
	
	return true;
end

local function ToFunctionName(name)
	return name:lower():gsub('^0x', 'n_0x'):gsub('_(%a)', string.upper):gsub('^(%a)', string.upper)
end

local function ToUniqueName(nativeName)
	local count = usedNatives[nativeName]
	if not count then
		usedNatives[nativeName] = 1
		return ''
	end
	
	usedNatives[nativeName] = count + 1
	return tostring(count)
end

local function GetWrapperTypeDirect(typ, isPointer)
	local wrapTyp = wrapperTypes[isPointer][typ]
	if wrapTyp then return wrapTyp end
	
	io.stderr:write('type is nil for: ', typ, ' as a ', (isPointer and 'pointer\n' or 'non-pointer\n'), debug.traceback(), '\n')
	return wrapperTypes[isPointer][valType]
end

local function GetWrapperType(typ, isPointer)
	local override = overrideParamTypes[isPointer][typ.name]
	if override then
		return GetWrapperTypeDirect(table.unpack(override)), override[2]
	end
	
	return GetWrapperTypeDirect(typ.nativeType, isPointer), isPointer
end

local function GetReturnType(typ)
	if not typ then return { 'void', {}, {}, {'void'} } end
	
	local typ, ptr = (overrideReturnTypes[typ.name] or typ.nativeType):match('(%w+)(%*?)')
	local isPointer = ptr ~= ''
	return GetWrapperTypeDirect(typ, isPointer), isPointer
end

local function ToNonLanguageName(str)
	return langWords[str] or str
end

local function TrimAndSanitizeXML(str)
	return trim(str):gsub('%W', { ['&'] = '&amp;', ['<'] = '&lt;', ['>'] = '&gt;', ['\n'] = '\n' .. t2 .. '/// ' })
end

local function FormatDocSectionXML(str, openTag, closeTag, allowEmptyTag)	
	if (str) then
		return t2..'/// '..openTag..'\n'
			..t2..'/// '..TrimAndSanitizeXML(str)..'\n'
			..t2..'/// '..closeTag..'\n';
	end
	
	return allowEmptyTag and t2 .. '/// ' .. openTag:sub(1, -2) .. ' />\n' or ''
end

local function FormatDocXML(native)
	local d = parseDocString(native)

	if d then
		local str = FormatDocSectionXML(d.summary, '<summary>', '</summary>')

		if d.hasParams then
			for _, v in ipairs(d.params) do
				str = str .. FormatDocSectionXML(v[2], '<param name="' .. ToNonLanguageName(v[1]) .. '">', '</param>', true)
			end
		end

		if d.returns then
			str = str .. FormatDocSectionXML(d.returns, '<returns>', '</returns>')
		end

		return str
	end
	
	return ''
end

local function LoadCompatibilityMethods()
	local compatFileName = arg[1]:gsub('.lua', '_'..gApiSet..'_compat.lua')
	local compatFile = loadfile(compatFileName)
	if compatFile then
		assert(compatFile)()
		io.stderr:write('\tIncluding compatibility methods, ', compatFileName, ' file is used.\n')
	end
end

local function GetCompatCode(to, from)
	local result = compatWrapperTypes[to]
	if result then
		result = result[from]
		if result then return result end
	end
	
	return nil
end

local function GetAndFormatCompatCode(to, from, index, pattern, value)
	local result = GetCompatCode(to, from)
	if result then
		result = result[index]
		if result then
			return result:gsub(pattern, value), true
		end
	end
	
	return value, false
end

local function PrintCompatibilityMethods(native, nativeReturnType, latestSignature)
	local compatMethods = compatibility[tonumber(native.hash)]
	if compatMethods then
		local length = #compatMethods
		
		local toMethodParam = latestSignature
		local toMethodParamLength = #latestSignature
		local toMethodReturn = latestSignature[1];
		
		for c = 1, length do
			local curMethod = compatMethods[c]
			curMethod[1] = overrideReturnTypes[curMethod[1]] or curMethod[1];
						
			if not ArraysEqual(curMethod, latestSignature) then				
				local hasResult = curMethod[1] ~= 'void'
				local wrapRetType = GetReturnType(hasResult and { name = curMethod[1], nativeType = curMethod[1] })				
				local resultType = GetOrDefault(wrapRetType[4], 1, valType)
				
				local init, body, post = '', (native.returns and nativeReturnType..'_' or 'void_')..native.hash..'(', ''
				
				if hasResult and native.returns then
					body = 'var __res = ' .. body
				end
				
				io.stdout:write('\n', t2, '[System.Obsolete("This method is obsolete", true)]\n')
				io.stdout:write(t2, 'public static unsafe ', resultType, ' ', (hasResult and resultType..'_' or 'void_'), native.hash, '(')
				
				-- setup all arguments
				local i, argn, comma = 2, math.min(#curMethod, toMethodParamLength), ''
				while i <= argn do
					local nativeType, toArgType, paramName = curMethod[i], toMethodParam[i], 'p'..tostring(i - 2)
					if toArgType then
						local compatCode = GetCompatCode(toArgType, nativeType)
						if compatCode then
							if compatCode[1] then init = init .. t3 .. compatCode[1]:gsub('{0}', paramName) .. ';\n' end
							body = body .. comma .. compatCode[2]:gsub('{0}', paramName)
							if compatCode[3] then post = post .. t3 .. compatCode[3]:gsub('{0}', paramName) .. ';\n' end
						else
							body = body .. comma .. paramName
						end
					else
						body = body .. comma .. paramName
					end
					
					local typeName, ptr = nativeType:match('(%w+)(%*?)')
					nativeType = GetWrapperTypeDirect(typeName, ptr == '*')[1]
					io.stdout:write(comma, nativeType, ' ', paramName)
					
					comma, i = ', ', i + 1
				end
				
				-- fill in blanks
				while i <= toMethodParamLength do
					local fromType, toArgType = 'default', toMethodParam[i]
					local compatCode = GetCompatCode(toArgType, fromType)
					if compatCode then
						if compatCode[1] then init = init .. t3 .. compatCode[1]:gsub('{0}', fromType) .. ';\n' end
						body = body .. comma .. compatCode[2]:gsub('{0}', fromType)
						if compatCode[3] then post = post .. t3 .. compatCode[3]:gsub('{0}', fromType) .. ';\n' end
					else
						body = body .. comma .. 'default'
					end
					comma, i = ', ', i + 1
				end
				
				io.stdout:write(')\n', t2, '{\n', init, t3, body, ');\n', post)
				
				-- returning and conversion
				if hasResult then
					local result, found = 'default', true
					if native.returns then
						result, found = GetAndFormatCompatCode(wrapRetType.compatType or resultType, toMethodReturn, 4, '{0}', '__res')
						if not result then io.stderr:write('No proper conversion found "', nativeReturnType, '" to "', resultType, '" in native ', native.name) end
					end
					
					io.stdout:write(t3, 'return ' .. result .. ';\n')
				end
				
				io.stdout:write(t2, '}\n')
			end
		end
	end	
end

local function PrintNativeMethod(native)
	local wrapRetType, writeVector3Out = GetReturnType(native.returns), false
	local retType = GetOrDefault(wrapRetType[4], 1, valType)
	local hasResult = retType ~= 'void'
	local funcSignature = { ToCType(wrapRetType.compatType or retType) }
	
	io.stdout:write(t2, '[System.Security.SecuritySafeCritical]\n', t2, 'public static unsafe ', retType, ' ',
		(hasResult and retType:gsub('ref ', '')..'_' or 'void_'), native.hash, '(')
	
	local monoStackBugFix = true;
	
	-- write parameters and append argument string for the ulong stack
	local argn, fixed, stack = 0, '', ''
	local comma, argSize = '', #native.arguments
	for i = 1, argSize do
		local arg = native.arguments[i]
		local name, wrapperType, asPointer = ToNonLanguageName(arg.name), GetWrapperType(arg.type, arg.pointer == true)
		
		io.stdout:write(comma, wrapperType[2][1]:gsub('{0}', name), '')
		
		local stackValue = wrapperType[2][2]:gsub('{0}', name)
		if stackValue ~= '0' then
			monoStackBugFix = false
		end
		
		stack = stack .. comma .. stackValue
		
		if wrapperType[2][3] then fixed = fixed..t3..wrapperType[2][3]:gsub('{0}', name)..'\n' end
		
		-- used for the compatibility system
		local cType = ToCType(wrapperType.compatType or wrapperType[2][1])
		if cType == 'Vector3' then
			table.insert(funcSignature, valType)
			table.insert(funcSignature, valType)
			table.insert(funcSignature, valType)
		else
			table.insert(funcSignature, cType)
		end
		
		argn = argn + (wrapperType.typeSize or 1)
		if wrapperType[1] == 'ref Vector3' then writeVector3Out = true end
		
		comma = ', '
	end
	
	local resultSize = hasResult and (wrapRetType.typeSize or 1) or -1
	if monoStackBugFix then
		io.stdout:write(')\n', t2, '{\n', fixed, t3, '{\n', t4, 'ulong* __data = stackalloc ulong[', math.max(resultSize, argn), '];\n')
	else
		io.stdout:write(')\n', t2, '{\n', fixed, t3, '{\n', t4, 'ulong* __data = stackalloc ulong[] { ', stack)
		
		-- make sure there's enough room to store the return types
		while argn < resultSize do
			io.stdout:write(comma, '0')
			comma, argn = ', ', argn + 1
		end
		
		io.stdout:write(' };\n')
	end
	
	-- invoke native
	io.stdout:write(t4, 'ScriptContext.InvokeNative')
	if writeVector3Out then io.stdout:write('OutVec3') end -- will call the Vector3 copy function, required to pass on the values
	io.stdout:write('(ref s_', native.hash, ', ', native.hash, ', __data, ', argn, ');\n')

	-- return result
	if hasResult then
		local resultInterpret = GetOrDefault(wrapRetType[4], 2, '*({1}*){0}');
		io.stdout:write(t4, 'return ', resultInterpret:gsub('{%d}', {['{0}'] = '__data', ['{1}'] = retType }), ';\n')
	end
	
	io.stdout:write(t3, '}\n', t2, '}\n\n', t2, 'private static UIntPtr s_', native.hash, ';\n')
	PrintCompatibilityMethods(native, retType, funcSignature)
	io.stdout:write('\n')
end

local function PrintWrappers(native)
	local parm, init, args, post, forw = '', '', '', '', '', ''
	
	local comma, argSize = '', #native.arguments;
	for i = 1, argSize do
		local arg = native.arguments[i];
		local name, wrapperType, ptr = ToNonLanguageName(arg.name), GetWrapperType(arg.type, arg.pointer == true)
		
		parm = parm .. comma .. wrapperType[1]..' '..name
		if wrapperType[3][1] then init = init .. t3 .. wrapperType[3][1]:gsub('{0}', name) .. ';\n' end
		args = args .. comma .. wrapperType[3][2]:gsub('{0}', name)
		if wrapperType[3][3] then post = post .. t3 .. wrapperType[3][3]:gsub('{%d}', {['{0}'] = name, ['{1}'] = GetOrDefault(wrapperType[4], 1, wrapperType[1]) }) .. ';\n' end
		forw = forw .. comma .. (ptr and 'ref ' or '') .. name
		
		comma = ', '
	end
	
	local wrapRetType = GetReturnType(native.returns);
	local doc, retType = FormatDocXML(native), GetOrDefault(wrapRetType[4], 3, wrapRetType[1])
	local nativeName, calleeRetType = native.displayName .. native.uniqueName, GetOrDefault(wrapRetType[4], 1, valType)
		
	io.stdout:write(doc, t2, 'public static ', retType, ' ', nativeName, '(', parm, ')\n', t2, '{\n', init, t3)
	
	if retType ~= 'void' then
		io.stdout:write('var __res = ')
		local convertType = GetOrDefault(wrapRetType[4], 4, 'N64.To_{1}({0})'):gsub('{.}', { ['{0}'] = '__res', ['{1}'] = retType })
		post = post .. t3 .. 'return ' .. convertType .. ';\n'
	end
	
	io.stdout:write('NativesImpl.', (native.returns and calleeRetType..'_' or 'void_'), native.hash, '(', args, ');\n', post, t2, '}\n\n')
	
	-- alias wrappers for native
	for _, alias in ipairs(native.aliases) do
		local aliasName = ToFunctionName(alias)
		if native.displayName ~= aliasName then
			io.stdout:write(doc, t2, 'public static ', retType, ' ', aliasName, '(', parm, ') => ', nativeName, '(', forw, ');\n\n')
		end
	end
end

local function PrintHash(native, iteration)
	if native.name:sub(1, 2) ~= '0x' then
		local doc = parseDocString(native)
		if doc ~= nil then
			io.stdout:write(FormatDocSectionXML(doc.summary, '<summary>', '</summary>'))
		end
		
		io.stdout:write(t2, native.name, ' = ', native.hash, ',\n')
		
		for _, alias in ipairs(native.aliases) do
			if alias:sub(1, 2) ~= '0x' then
				io.stdout:write(t2, '[System.Obsolete("Deprecated name, use ', native.name, ' instead")]\n')
				io.stdout:write(t2, alias, ' = ', native.hash, ',\n')
			end
		end
	end
end

local function PrintPointerArgumentSafety(natives)
	io.stdout:write(t1, 'internal static partial class PointerArgumentSafety\n', t1, '{\n',
		t2, 'static internal System.Collections.Generic.Dictionary<ulong, byte> s_nativeInfo = new System.Collections.Generic.Dictionary<ulong, byte>\n', t2, '{\n')

	local primitive_t = valType;
	local func_t = wrapperTypes[false]['func'][4][1];
	local vector3_t = wrapperTypes[false]['Vector3'][4][1];
	local string_t = wrapperTypes[false]['string'][4][1];
	local object_t = wrapperTypes[false]['object'][4][1];

	for _, native in pairs(natives) do
		if native.returns then
			local typ = GetOrDefault(GetReturnType(native.returns)[4], 1, primitive_t)
			
			io.stdout:write(t3, '[', native.hash, '] = ')
			
			if native.name == 'INVOKE_FUNCTION_REFERENCE' then
				io.stdout:write('0x80')
			elseif typ == primitive_t then
				io.stdout:write('0x01')
			elseif typ == vector3_t then
				io.stdout:write('0x03')
			elseif typ == string_t then
				io.stdout:write('0x40')
			elseif typ == object_t then
				io.stdout:write('0x20')
			else
				io.stdout:write('0x00')
			end
			
			io.stdout:write(', // ', (native.ns or 'UNKNOWN'), '/', native.name, '\n')
		end
	end
	
	io.stdout:write(t2, '};\n}\n')
end


-- =============================
-- ==  Execution starts here  ==
-- =============================

-- sort the natives table
local _natives, _nativesShared, nativeCount = {}, {}, #natives

for i = 1, nativeCount do
	local native = natives[i]
	if matchApiSet(native) then
		native.displayName = ToFunctionName(native.name)
		native.uniqueName = ToUniqueName(native.displayName)
		
		for _, v in ipairs(native.apiset) do
			if (v == 'shared') then
				table.insert(_nativesShared, native)
				break
			end
		end
		
		table.insert(_natives, native)
	end
end

nativeCount = #_natives
nativeSharedCount = #_nativesShared

-- sort alphabetically
table.sort(_natives, function(a, b) return a.displayName < b.displayName end)
table.sort(_nativesShared, function(a, b) return a.displayName < b.displayName end)

LoadCompatibilityMethods()

-- get top constant
local topConstant = 'Unknown'
local productName = gApiSet == 'server' and 'server' or arg[1]:match('_(%w+)%..+$')
if productName then
	local product = PRODUCTS[productName]
	if product then
		topConstant = product[1];
		productName = product[2];
	else
		local err = 'Error, couldn\'t find game "'..productName..'" in the PRODUCTS list, please pick one from the list in "codegen_out_cs_v2.lua"'
		topConstant = 'Unknown // '..err
		io.stderr:write(err)
	end
else
	local err = 'Error, couldn\'t get game name from "'..arg[1]..'", doesn\'t match pattern "_(%w+)%..+$"'
	topConstant = 'Unknown // '..err
	productName = 'Unknown'
	io.stderr:write(err)
end

io.stdout:write('#if ', topConstant, '\nusing System;\nusing System.ComponentModel;\nusing CitizenFX.Core;\nusing CitizenFX.Core.Native;\n\n')

-- ignore missing argument documentation warnings
io.stdout:write('#pragma warning disable CS1573\n\n')

-- versioning
io.stdout:write('#if NATIVE_WRAPPER_USE_VERSION\n')
io.stdout:write('[assembly: System.Reflection.AssemblyVersion("', table.concat(version, '.'), '")]\n')
io.stdout:write('[assembly: System.Reflection.AssemblyFileVersion("', table.concat(version, '.'), '")]\n')
io.stdout:write('#endif\n\n')

-- product specfific natives
do 
	io.stdout:write('namespace CitizenFX.', productName, '.Native\n{\n')

	-- write enums
	io.stdout:write('\n#if NATIVE_HASHES_INCLUDE\n', t1, 'public enum Hash : ulong\n', t1 ,'{\n')
	for i = 1, nativeCount do
		PrintHash(_natives[i])
	end
	io.stdout:write(t1, '}\n#endif\n')

	-- native base methods
	io.stdout:write('\n#if NATIVE_IMPL_INCLUDE\n',
		t1, '[EditorBrowsable(EditorBrowsableState.Never)]\n',
		t1, 'public static class NativesImpl\n', t1 ,'{\n')
	
	for i = 1, nativeCount do
		PrintNativeMethod(_natives[i])
	end
	io.stdout:write(t1, '}\n#endif\n')

	-- native wrappers
	io.stdout:write('\n#if NATIVE_WRAPPERS_INCLUDE\n', t1, 'public static partial class Natives\n', t1 ,'{\n')
	for i = 1, nativeCount do
		PrintWrappers(_natives[i])
	end
	io.stdout:write(t1, '}\n#endif\n}\n')
end

-- shared natives
do
	io.stdout:write('\n#if NATIVE_SHARED_INCLUDE\nnamespace CitizenFX.Shared.Native\n{\n')
	
	-- hashes
	io.stdout:write(t1, 'public enum Hash : ulong\n', t1 ,'{\n')
	for i = 1, nativeSharedCount do
		PrintHash(_nativesShared[i])
	end
	io.stdout:write(t1, '}\n\n')
	
	-- implementation
	io.stdout:write(t1, '[EditorBrowsable(EditorBrowsableState.Never)]\n', t1, 'public static class NativesImpl\n', t1 ,'{\n')
	
	for i = 1, nativeSharedCount do
		PrintNativeMethod(_nativesShared[i])
	end
	io.stdout:write(t1, '}\n\n')
	
	-- wrappers
	io.stdout:write(t1, 'public static partial class Natives\n', t1 ,'{\n')
	for i = 1, nativeSharedCount do
		PrintWrappers(_nativesShared[i])
	end
	io.stdout:write(t1, '}\n\n')
	
	-- Pointer Argument Safety (PAS) bits
	PrintPointerArgumentSafety(_natives)
	
	io.stdout:write('}\n#endif\n');
end

io.stdout:write('#endif\n')
