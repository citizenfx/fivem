-- setup environment for the codegen'd file to execute in
codeEnvironment = {

}

types = {}
natives = {}
rpcNatives = {}
local curType
local curNative
local codeEnvironment = codeEnvironment

local json = require('dkjson')
local cfx = require('cfx')

function codeEnvironment.type(typeName)
	-- create a new type entry
	types[typeName] = {
		name = typeName
	}

	-- set a local
	local typeEntry = types[typeName]

	-- set a closure named after the type in the code environment
	codeEnvironment[typeName] = function(argumentName)
		local t = {
			type = typeEntry,
			name = argumentName
		}

		return setmetatable(t, {
			__call = function(t, list)
				t.annotations = json.decode(list)
				return t
			end
		})
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
	curType.parent = types[base]
end

function codeEnvironment.nativeType(typeName)
	curType.nativeType = typeName
end

function codeEnvironment.subType(typeName)
	curType.subType = typeName
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
		apiset = {},
		hash = ("0x%x"):format(cfx.hash(nativeName:lower())),
		arguments = {},
		aliases = {}
	}

	table.insert(natives, native)

	-- set the current native to said native
	curNative = native
end

function codeEnvironment.doc(docString)
	if not curNative then
		return
	end

	curNative.doc = docString
end

function codeEnvironment.ns(nsString)
	curNative.ns = nsString
end

function codeEnvironment.hash(hashString)
	curNative.hash = hashString
end

function codeEnvironment.alias(name)
	table.insert(curNative.aliases, name)
end

function codeEnvironment.jhash(hash)
	curNative.jhash = hash
end

function codeEnvironment.arguments(list)
	curNative.arguments = list
end

function codeEnvironment.returns(typeName)
	if type(typeName) == 'table' then
		curNative.returns = typeName
	else
		curNative.returns = types[typeName]
	end
end

function codeEnvironment.game(gameName)
	curNative.game = gameName
end

function codeEnvironment.annotations(list)
	curNative.annotations = json.decode(list)
end

function codeEnvironment.apiset(setName)
	table.insert(curNative.apiset, setName)
end

-- load the definition file
function loadDefinition(filename)
	local chunk = loadfile(filename, 't', codeEnvironment)

	chunk()
end

-- load an rpc definition file
local rpcEnvironment = {}

local function isType(type, baseName)
	local valid = false

	repeat
		valid = valid or type.name == baseName

		type = type.parent
	until not type

	return valid
end

local function isPrimitive(type)
	return type.name == 'Any' or type.name == 'uint' or type.name == 'int' or
		type.name == 'Hash' or type.name == 'charPtr' or type.name == 'float' or
		type.name == 'vector3' or type.name == 'BOOL'
end

local function getNative(nativeName)
	-- get the native
	local n

	for k, v in ipairs(natives) do
		if v.name == nativeName and (#v.apiset == 0 or v.apiset[1] == 'client') then
			n = v
			break
		end
	end

	return n
end

local function getServerNative(nativeName)
	-- get the native
	local n
	
	for k, v in ipairs(natives) do
		if v.name == nativeName and #v.apiset > 0 and v.apiset[1] == 'server' then
			n = v
			break
		end
	end

	return n
end

local function removeServerNative(nativeName)
	local n = getServerNative(nativeName)

	if n then
		for k, v in pairs(natives) do
			if v == n then
				table.remove(natives, k)
				break
			end
		end
	end
end

function rpcEnvironment.context_rpc(nativeName)
	local n = getNative(nativeName)

	if not n then
		return
	end

	if #n.arguments == 0 then
		error('Context natives are required to have arguments.')
	end

	local rn = {}

	-- generate native arguments
	local ctx
	local args = {}

	for k, v in ipairs(n.arguments) do
		-- is this an entity?
		if isType(v.type, 'Entity') then
			-- if not the context, this is the context
			if not ctx then
				-- TODO: devise a way to disallow players for natives that need to
				ctx = { idx = k - 1, type = 'Entity' }
			end

			table.insert(args, {
				translate = true,
				type = 'Entity'
			})
		elseif v.type.name == 'Player' then
			-- if not the context, this is the context
			if not ctx then
				ctx = { idx = k - 1, type = 'Player' }
			end

			table.insert(args, {
				translate = true,
				type = 'Player'
			})
		elseif not isPrimitive(v.type) then
			if not ctx then
				if v.pointer then
					ctx = { idx = k - 1, type = 'ObjDel' }
					
					table.insert(args, {
						translate = true,
						type = 'ObjDel'
					})
				else
					ctx = { idx = k - 1, type = 'ObjRef' }
					
					table.insert(args, {
						translate = true,
						type = 'ObjRef'
					})
				end
			else
				error(('Type %s is not supported for RPC natives.'):format(v.type.name))
			end
		else
			table.insert(args, {
				type = v.type.name
			})
		end
	end

	if not ctx then
		error('Context RPC natives are required to have a context.')
	end

	rn.ctx = ctx
	rn.args = args

	removeServerNative(nativeName)
	codeEnvironment.native(nativeName)
		codeEnvironment.arguments(n.arguments)
		codeEnvironment.apiset('server')
		codeEnvironment.returns('void')
		curNative.rpc = true
		curNative.ogHash = n.hash

		if n.doc then
			codeEnvironment.doc(n.doc)
		end

	rn.hash = n.hash
	rn.name = nativeName

	rn.type = 'ctx'

	table.insert(rpcNatives, rn)
	
	return function(tbl)
		-- single getter
		if type(tbl) == 'table' and #tbl == 1 then
			tbl = { getter = tbl[1] }
		end
		
		-- if we need to define a getter
		if tbl.getter then
			local rvArg = n.arguments[2].type
			
			local function isFloat(arg)
				return arg.type.nativeType == 'float'
			end
			
			-- check if this is perhaps a vector3
			if #n.arguments >= 4 and isFloat(n.arguments[2]) and isFloat(n.arguments[3]) and isFloat(n.arguments[4]) then
				rvArg = types['Vector3']
			end
			
			rn.getter = {
				returnType = rvArg.name,
				returnArgStart = 1,
				name = tbl.getter
			}
			
			local cxtRef
			
			if ctx.type == 'Player' then
				cxtRef = codeEnvironment.Player
			elseif ctx.type == 'Entity' then
				cxtRef = codeEnvironment.Entity
			elseif ctx.type == 'ObjRef' then
				cxtRef = codeEnvironment.int
			end
			
			if not getServerNative(tbl.getter) then
				codeEnvironment.native(tbl.getter)
					codeEnvironment.arguments {
						cxtRef 'self'
					}
					codeEnvironment.apiset('server')
					codeEnvironment.returns(rvArg)
			end
		end
	end
end

function rpcEnvironment.entity_rpc(nativeName)
	local n = getNative(nativeName)

	if not n then
		return
	end

	if not n.returns then
		error('Entity natives are required to return an entity.')
	end

	if not isType(n.returns, 'Entity') then
		error('Entity natives are required to return an entity.')
	end

	local rn = {}

	-- generate native arguments
	local args = {}

	for k, v in ipairs(n.arguments) do
		-- is this an entity?
		if isType(v.type, 'Entity') then
			table.insert(args, {
				translate = true,
				type = 'Entity'
			})
		elseif v.type.name == 'Player' then
			table.insert(args, {
				translate = true,
				type = 'Player'
			})
		elseif not isPrimitive(v.type) then
			error(('Type %s is not supported for RPC natives.'):format(v.type.name))
		else
			table.insert(args, {
				type = v.type.name
			})
		end
	end

	rn.args = args

	removeServerNative(nativeName)
	codeEnvironment.native(nativeName)
		codeEnvironment.arguments(n.arguments)
		codeEnvironment.apiset('server')
		codeEnvironment.returns('Entity')
		curNative.rpc = true
		curNative.ogHash = n.hash

		if n.doc then
			codeEnvironment.doc(n.doc)
		end

	rn.hash = n.hash
	rn.name = nativeName

	rn.type = 'entity'

	table.insert(rpcNatives, rn)
end

function rpcEnvironment.object_rpc(nativeName)
	local n = getNative(nativeName)

	if not n then
		return
	end

	if not n.returns then
		error('Object natives are required to return an object.')
	end

	if isType(n.returns, 'Entity') then
		error('Object natives are not entity natives.')
	end

	local rn = {}

	-- generate native arguments
	local args = {}

	for k, v in ipairs(n.arguments) do
		-- is this an entity?
		if isType(v.type, 'Entity') then
			table.insert(args, {
				translate = true,
				type = 'Entity'
			})
		elseif v.type.name == 'Player' then
			table.insert(args, {
				translate = true,
				type = 'Player'
			})
		elseif not isPrimitive(v.type) then
			error(('Type %s is not supported for RPC natives.'):format(v.type.name))
		else
			table.insert(args, {
				type = v.type.name
			})
		end
	end

	rn.args = args
	
	removeServerNative(nativeName)
	codeEnvironment.native(nativeName)
		codeEnvironment.arguments(n.arguments)
		codeEnvironment.apiset('server')
		codeEnvironment.returns(n.returns)
		curNative.rpc = true
		curNative.ogHash = n.hash

		if n.doc then
			codeEnvironment.doc(n.doc)
		end

	rn.hash = n.hash
	rn.name = nativeName

	rn.type = 'object'

	table.insert(rpcNatives, rn)
end

function loadRpcDefinition(filename)
	local chunk = loadfile(filename, 't', rpcEnvironment)

	chunk()
end

function trim(s)
	if not s then
		return nil
	end

	return s:gsub("^%s*(.-)%s*$", "%1")
end

function parseDocString(native)
	local docString = native.doc

	if not docString then
		return nil
	end

	local summary = trim(docString:match("<summary>(.+)</summary>"))
	local params = docString:gmatch("<param name=\"([^\"]+)\">([^<]*)</param>")
	local returns = docString:match("<returns>(.+)</returns>")

	if not summary then
		summary = ''
	end

	summary = trim(summary:gsub('^```(.+)```$', '%1'))

	local paramsData = {}
	local hasParams = false

	for k, v in params do
		table.insert(paramsData, { k, v })
		hasParams = true
	end

	if summary == '' then
		if not hasParams and trim(returns) == '' then
			return nil
		end

		summary = native.name
	end

	return {
		summary = summary,
		params = paramsData,
		hasParams = hasParams,
		returns = returns
	}
end

local gFilterRpc = false
local gApiSet = 'server'
local ourGame = 'gta5'

function matchApiSet(native)
	local game = native.game

	if ourGame and native.game and native.game ~= ourGame then
		return false
	end

	if gFilterRpc and not native.rpc then
		return false
	end

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

local outputType = 'lua'
local globalNatives = false

if #arg > 0 then
	if arg[1]:match('gta_universal') then
		arg[1] = 'inp/natives_global.lua'
	end

	if arg[1]:match('rdr3_universal') then
		arg[1] = 'inp/natives_rdr3.lua'
	end

	if arg[1]:match('ny_universal') then
		arg[1] = 'inp/natives_ny.lua'
	end

	if arg[1]:match('rdr3') then
		ourGame = 'rdr3'
	end

	if arg[1]:match('_ny') then
		ourGame = 'ny'
	end

	loadDefinition(arg[1])

	if arg[1]:match('natives_global') or arg[1]:match('natives_rdr3') or arg[1]:match('natives_ny') then
		globalNatives = true
	end

	gApiSet = 'client'
end

if #arg > 1 then
	outputType = arg[2]
end

if #arg > 2 then
	gApiSet = arg[3]
end

if #arg > 3 then
	if arg[4] == 'rpc' then
		gFilterRpc = true
	end
end

-- server has Player be charPtr, not int
if gApiSet == 'server' then
	types['Player'].nativeType = 'string'
end

if not gFilterRpc then
	loadDefinition 'inp/natives_cfx.lua'
end

if not globalNatives then
	loadDefinition 'codegen_dlc_natives.lua'
end

if arg[1]:match('ves_glo') then
	loadRpcDefinition 'rpc_spec_natives.lua'
end

_natives = {}

for _, v in ipairs(natives) do
	table.insert(_natives, v)
end

table.sort(_natives, function(a, b)
	return a.name < b.name
end)

_G.gApiSet = gApiSet

dofile('codegen_out_' .. outputType .. '.lua')
