local type = type
local error = error
local pairs = pairs
local rawget = rawget
local tonumber = tonumber
local getmetatable = getmetatable
local setmetatable = setmetatable

local debug = debug
local debug_getinfo = debug.getinfo

local table_pack = table.pack
local table_unpack = table.unpack
local table_insert = table.insert

local coroutine_create = coroutine.create
local coroutine_yield = coroutine.yield
local coroutine_resume = coroutine.resume
local coroutine_status = coroutine.status
local coroutine_running = coroutine.running
local coroutine_close = coroutine.close or (function(c) end) -- 5.3 compatibility

--[[ Custom extensions --]]
local msgpack = msgpack
local msgpack_pack = msgpack.pack
local msgpack_unpack = msgpack.unpack
local msgpack_pack_args = msgpack.pack_args

local Citizen = Citizen
local Citizen_SubmitBoundaryStart = Citizen.SubmitBoundaryStart
local Citizen_InvokeFunctionReference = Citizen.InvokeFunctionReference
local GetGameTimer = GetGameTimer
local ProfilerEnterScope = ProfilerEnterScope
local ProfilerExitScope = ProfilerExitScope

local hadThread = false
local curTime = 0
local hadProfiler = false
local isDuplicityVersion = IsDuplicityVersion()

local function _ProfilerEnterScope(name)
	if hadProfiler then
		ProfilerEnterScope(name)
	end
end

local function _ProfilerExitScope()
	if hadProfiler then
		ProfilerExitScope()
	end
end

-- setup msgpack compat
msgpack.set_string('string_compat')
msgpack.set_integer('unsigned')
msgpack.set_array('without_hole')
msgpack.setoption('empty_table_as_array', true)

-- setup json compat
json.version = json._VERSION -- Version compatibility
json.setoption("empty_table_as_array", true)
json.setoption('with_hole', true)

-- temp
local _in = Citizen.InvokeNative

local function FormatStackTrace()
	return _in(`FORMAT_STACK_TRACE` & 0xFFFFFFFF, nil, 0, Citizen.ResultAsString())
end

local boundaryIdx = 1

local function dummyUseBoundary(idx)
	return nil
end

local function getBoundaryFunc(bfn, bid)
	return function(fn, ...)
		local boundary = bid
		if not boundary then
			boundary = boundaryIdx + 1
			boundaryIdx = boundary
		end
		
		bfn(boundary, coroutine_running())

		local wrap = function(...)
			dummyUseBoundary(boundary)
			
			local v = table_pack(fn(...))
			return table_unpack(v)
		end
		
		local v = table_pack(wrap(...))
		
		bfn(boundary, nil)
		
		return table_unpack(v)
	end
end

local runWithBoundaryStart = getBoundaryFunc(Citizen.SubmitBoundaryStart)
local runWithBoundaryEnd = getBoundaryFunc(Citizen.SubmitBoundaryEnd)

local AwaitSentinel = Citizen.AwaitSentinel()
Citizen.AwaitSentinel = nil

function Citizen.Await(promise)
	local coro = coroutine_running()
	assert(coro, "Current execution context is not in the scheduler, you should use CreateThread / SetTimeout or Event system (AddEventHandler) to be able to Await")

	if promise.state == 0 then
		local reattach = coroutine_yield(AwaitSentinel)
		promise:next(reattach, reattach)
		coroutine_yield()
	end

	if promise.state == 2 or promise.state == 4 then
		error(promise.value, 2)
	end

	return promise.value
end

Citizen.SetBoundaryRoutine(function(f)
	boundaryIdx = boundaryIdx + 1

	local bid = boundaryIdx
	return bid, function()
		return runWithBoundaryStart(f, bid)
	end
end)

-- root-level alias (to prevent people from calling the game's function accidentally)
Wait = Citizen.Wait
CreateThread = Citizen.CreateThread
SetTimeout = Citizen.SetTimeout

--[[

	Event handling

]]

local eventHandlers = {}
local deserializingNetEvent = false

Citizen.SetEventRoutine(function(eventName, eventPayload, eventSource)
	-- set the event source
	local lastSource = _G.source
	_G.source = eventSource

	-- try finding an event handler for the event
	local eventHandlerEntry = eventHandlers[eventName]

	-- deserialize the event structure (so that we end up adding references to delete later on)
	local data = msgpack_unpack(eventPayload)

	if eventHandlerEntry and eventHandlerEntry.handlers then
		-- if this is a net event and we don't allow this event to be triggered from the network, return
		if eventSource:sub(1, 3) == 'net' then
			if not eventHandlerEntry.safeForNet then
				Citizen.Trace('event ' .. eventName .. " was not safe for net\n")

				_G.source = lastSource
				return
			end

			deserializingNetEvent = { source = eventSource }
			_G.source = tonumber(eventSource:sub(5))
		elseif isDuplicityVersion and eventSource:sub(1, 12) == 'internal-net' then
			deserializingNetEvent = { source = eventSource:sub(10) }
			_G.source = tonumber(eventSource:sub(14))
		end

		-- return an empty table if the data is nil
		if not data then
			data = {}
		end

		-- reset serialization
		deserializingNetEvent = nil

		-- if this is a table...
		if type(data) == 'table' then
			-- loop through all the event handlers
			for k, handler in pairs(eventHandlerEntry.handlers) do
				local handlerFn = handler
				local handlerMT = getmetatable(handlerFn)

				if handlerMT and handlerMT.__call then
					handlerFn = handlerMT.__call
				end

				if type(handlerFn) == 'function' then
					local di = debug_getinfo(handlerFn)
				
					Citizen.CreateThreadNow(function()
						handler(table_unpack(data))
					end, ('event %s [%s[%d..%d]]'):format(eventName, di.short_src, di.linedefined, di.lastlinedefined))
				end
			end
		end
	end

	_G.source = lastSource
end)

local stackTraceBoundaryIdx

Citizen.SetStackTraceRoutine(function(bs, ts, be, te)
	if not ts then
		ts = runningThread
	end

	local t
	local n = 0
	
	local frames = {}
	local skip = false
	
	if bs then
		skip = true
	end

	repeat
		if ts then
			t = debug_getinfo(ts, n, 'nlfS')
		else
			t = debug_getinfo(n + 1, 'nlfS')
		end

		if t then
			if t.name == 'wrap' and t.source == '@citizen:/scripting/lua/scheduler.lua' then
				if not stackTraceBoundaryIdx then
					local b, v
					local u = 1
					
					repeat
						b, v = debug.getupvalue(t.func, u)
						
						if b == 'boundary' then
							break
						end
						
						u = u + 1
					until not b
					
					stackTraceBoundaryIdx = u
				end
				
				local _, boundary = debug.getupvalue(t.func, stackTraceBoundaryIdx)
				
				if boundary == bs then
					skip = false
				end
				
				if boundary == be then
					break
				end
			end
			
			if not skip then
				if t.source and t.source:sub(1, 1) ~= '=' and t.source:sub(1, 10) ~= '@citizen:/' then
					table_insert(frames, {
						file = t.source:sub(2),
						line = t.currentline,
						name = t.name or '[global chunk]'
					})
				end
			end
		
			n = n + 1
		end
	until not t
	
	return msgpack_pack(frames)
end)

local eventKey = 10

function AddEventHandler(eventName, eventRoutine)
	local tableEntry = eventHandlers[eventName]

	if not tableEntry then
		tableEntry = { }

		eventHandlers[eventName] = tableEntry
	end

	if not tableEntry.handlers then
		tableEntry.handlers = { }
	end

	eventKey = eventKey + 1
	tableEntry.handlers[eventKey] = eventRoutine

	RegisterResourceAsEventHandler(eventName)

	return {
		key = eventKey,
		name = eventName
	}
end

function RemoveEventHandler(eventData)
	if not eventData or not eventData.key or not eventData.name then
		error('Invalid event data passed to RemoveEventHandler()', 2)
	end

	-- remove the entry
	eventHandlers[eventData.name].handlers[eventData.key] = nil
end

local ignoreNetEvent = {
	['__cfx_internal:commandFallback'] = true,
}

function RegisterNetEvent(eventName, cb)
	if not ignoreNetEvent[eventName] then
		local tableEntry = eventHandlers[eventName]

		if not tableEntry then
			tableEntry = { }

			eventHandlers[eventName] = tableEntry
		end

		tableEntry.safeForNet = true
	end

	if cb then
		return AddEventHandler(eventName, cb)
	end
end

function TriggerEvent(eventName, ...)
	local payload = msgpack_pack_args(...)

	return runWithBoundaryEnd(function()
		return TriggerEventInternal(eventName, payload, payload:len())
	end)
end

if isDuplicityVersion then
	function TriggerClientEvent(eventName, playerId, ...)
		local payload = msgpack_pack_args(...)

		return TriggerClientEventInternal(eventName, playerId, payload, payload:len())
	end
	
	function TriggerLatentClientEvent(eventName, playerId, bps, ...)
		local payload = msgpack_pack_args(...)

		return TriggerLatentClientEventInternal(eventName, playerId, payload, payload:len(), tonumber(bps))
	end

	RegisterServerEvent = RegisterNetEvent
	RconPrint = Citizen.Trace
	GetPlayerEP = GetPlayerEndpoint
	RconLog = function() end

	function GetPlayerIdentifiers(player)
		local numIds = GetNumPlayerIdentifiers(player)
		local t = {}

		for i = 0, numIds - 1 do
			table_insert(t, GetPlayerIdentifier(player, i))
		end

		return t
	end

	function GetPlayerTokens(player)
		local numIds = GetNumPlayerTokens(player)
		local t = {}

		for i = 0, numIds - 1 do
			table_insert(t, GetPlayerToken(player, i))
		end

		return t
	end

	function GetPlayers()
		local num = GetNumPlayerIndices()
		local t = {}

		for i = 0, num - 1 do
			table_insert(t, GetPlayerFromIndex(i))
		end

		return t
	end

	local httpDispatch = {}
	AddEventHandler('__cfx_internal:httpResponse', function(token, status, body, headers, errorData)
		if httpDispatch[token] then
			local userCallback = httpDispatch[token]
			httpDispatch[token] = nil
			userCallback(status, body, headers, errorData)
		end
	end)

	function PerformHttpRequest(url, cb, method, data, headers, options)
		local followLocation = true
		
		if options and options.followLocation ~= nil then
			followLocation = options.followLocation
		end
	
		local t = {
			url = url,
			method = method or 'GET',
			data = data or '',
			headers = headers or {},
			followLocation = followLocation
		}

		local id = PerformHttpRequestInternalEx(t)

		if id ~= -1 then
			httpDispatch[id] = cb
		else
			cb(0, nil, {}, 'Failure handling HTTP request')
		end
	end

	function PerformHttpRequestAwait(url, method, data, headers, options)
		local p = promise.new()
		PerformHttpRequest(url, function(...)
			p:resolve({...})
		end, method, data, headers, options)

		Citizen.Await(p)
		return table.unpack(p.value)
	end
else
	function TriggerServerEvent(eventName, ...)
		local payload = msgpack_pack_args(...)

		return TriggerServerEventInternal(eventName, payload, payload:len())
	end
	
	function TriggerLatentServerEvent(eventName, bps, ...)
		local payload = msgpack_pack_args(...)

		return TriggerLatentServerEventInternal(eventName, payload, payload:len(), tonumber(bps))
	end
end

local funcRefs = {}
local funcRefIdx = 0

local function MakeFunctionReference(func)
	local thisIdx = funcRefIdx

	funcRefs[thisIdx] = {
		func = func,
		refs = 0
	}

	funcRefIdx = funcRefIdx + 1

	local refStr = Citizen.CanonicalizeRef(thisIdx)
	return refStr
end

function Citizen.GetFunctionReference(func)
	if type(func) == 'function' then
		return MakeFunctionReference(func)
	elseif type(func) == 'table' and rawget(func, '__cfx_functionReference') then
		return MakeFunctionReference(function(...)
			return func(...)
		end)
	end

	return nil
end

local function doStackFormat(err)
	local fst = FormatStackTrace()
	
	-- already recovering from an error
	if not fst then
		return nil
	end

	return string.format('^1SCRIPT ERROR: %s^7\n%s', err or '', fst)
end

Citizen.SetCallRefRoutine(function(refId, argsSerialized)
	local refPtr = funcRefs[refId]

	if not refPtr then
		Citizen.Trace('Invalid ref call attempt: ' .. refId .. "\n")

		return msgpack_pack(nil)
	end
	
	local ref = refPtr.func

	local err
	local retvals = false
	local cb = {}
	
	local di = debug_getinfo(ref)

	local waited = Citizen.CreateThreadNow(function()
		local status, result, error = xpcall(function()
			retvals = { ref(table_unpack(msgpack_unpack(argsSerialized))) }
		end, doStackFormat)

		if not status then
			err = result or ''
		end

		if cb.cb then
			cb.cb(retvals, err)
		elseif err then
			Citizen.Trace(err)
		end
	end, ('ref call [%s[%d..%d]]'):format(di.short_src, di.linedefined, di.lastlinedefined))

	if not waited then
		if err then
			return msgpack_pack(nil)
		end

		return msgpack_pack(retvals)
	else
		return msgpack_pack({{
			__cfx_async_retval = function(rvcb)
				cb.cb = rvcb
			end
		}})
	end
end)

Citizen.SetDuplicateRefRoutine(function(refId)
	local ref = funcRefs[refId]

	if ref then
		ref.refs = ref.refs + 1

		return refId
	end

	return -1
end)

Citizen.SetDeleteRefRoutine(function(refId)
	local ref = funcRefs[refId]
	
	if ref then
		ref.refs = ref.refs - 1
		
		if ref.refs <= 0 then
			funcRefs[refId] = nil
		end
	end
end)

-- RPC REQUEST HANDLER
local InvokeRpcEvent

if GetCurrentResourceName() == 'sessionmanager' then
	local rpcEvName = ('__cfx_rpcReq')

	RegisterNetEvent(rpcEvName)

	AddEventHandler(rpcEvName, function(retEvent, retId, refId, args)
		local source = source

		local eventTriggerFn = TriggerServerEvent
		
		if isDuplicityVersion then
			eventTriggerFn = function(name, ...)
				TriggerClientEvent(name, source, ...)
			end
		end

		local returnEvent = function(args, err)
			eventTriggerFn(retEvent, retId, args, err)
		end

		local function makeArgRefs(o)
			if type(o) == 'table' then
				for k, v in pairs(o) do
					if type(v) == 'table' and rawget(v, '__cfx_functionReference') then
						o[k] = function(...)
							return InvokeRpcEvent(source, rawget(v, '__cfx_functionReference'), {...})
						end
					end

					makeArgRefs(v)
				end
			end
		end

		makeArgRefs(args)

		runWithBoundaryEnd(function()
			local payload = Citizen_InvokeFunctionReference(refId, msgpack_pack(args))

			if #payload == 0 then
				returnEvent(false, 'err')
				return
			end

			local rvs = msgpack_unpack(payload)

			if type(rvs[1]) == 'table' and rvs[1].__cfx_async_retval then
				rvs[1].__cfx_async_retval(returnEvent)
			else
				returnEvent(rvs)
			end
		end)
	end)
end

local rpcId = 0
local rpcPromises = {}
local playerPromises = {}

-- RPC REPLY HANDLER
local repName = ('__cfx_rpcRep:%s'):format(GetCurrentResourceName())

RegisterNetEvent(repName)

AddEventHandler(repName, function(retId, args, err)
	local promise = rpcPromises[retId]
	rpcPromises[retId] = nil

	-- remove any player promise for us
	for k, v in pairs(playerPromises) do
		v[retId] = nil
	end

	if promise then
		if args then
			promise:resolve(args[1])
		elseif err then
			promise:reject(err)
		end
	end
end)

if isDuplicityVersion then
	AddEventHandler('playerDropped', function(reason)
		local source = source

		if playerPromises[source] then
			for k, v in pairs(playerPromises[source]) do
				local p = rpcPromises[k]

				if p then
					p:reject('Player dropped: ' .. reason)
				end
			end
		end

		playerPromises[source] = nil
	end)
end

local EXT_FUNCREF = 10
local EXT_LOCALFUNCREF = 11

msgpack.extend_clear(EXT_FUNCREF, EXT_LOCALFUNCREF)

-- RPC INVOCATION
InvokeRpcEvent = function(source, ref, args)
	if not coroutine_running() then
		error('RPC delegates can only be invoked from a thread.', 2)
	end

	local src = source

	local eventTriggerFn = TriggerServerEvent

	if isDuplicityVersion then
		eventTriggerFn = function(name, ...)
			TriggerClientEvent(name, src, ...)
		end
	end

	local p = promise.new()
	local asyncId = rpcId
	rpcId = rpcId + 1

	local refId = ('%d:%d'):format(GetInstanceId(), asyncId)

	eventTriggerFn('__cfx_rpcReq', repName, refId, ref, args)

	-- add rpc promise
	rpcPromises[refId] = p

	-- add a player promise
	if not playerPromises[src] then
		playerPromises[src] = {}
	end

	playerPromises[src][refId] = true

	return Citizen.Await(p)
end

local funcref_mt = nil

funcref_mt = msgpack.extend({
	__gc = function(t)
		DeleteFunctionReference(rawget(t, '__cfx_functionReference'))
	end,

	__index = function(t, k)
		error('Cannot index a funcref', 2)
	end,

	__newindex = function(t, k, v)
		error('Cannot set indexes on a funcref', 2)
	end,

	__call = function(t, ...)
		local netSource = rawget(t, '__cfx_functionSource')
		local ref = rawget(t, '__cfx_functionReference')

		if not netSource then
			local args = msgpack_pack_args(...)

			-- as Lua doesn't allow directly getting lengths from a data buffer, and _s will zero-terminate, we have a wrapper in the game itself
			local rv = runWithBoundaryEnd(function()
				return Citizen_InvokeFunctionReference(ref, args)
			end)
			local rvs = msgpack_unpack(rv)

			-- handle async retvals from refs
			if rvs and type(rvs[1]) == 'table' and rawget(rvs[1], '__cfx_async_retval') and coroutine_running() then
				local p = promise.new()

				rvs[1].__cfx_async_retval(function(r, e)
					if r then
						p:resolve(r)
					elseif e then
						p:reject(e)
					end
				end)

				return table_unpack(Citizen.Await(p))
			end

			if not rvs then
				error()
			end

			return table_unpack(rvs)
		else
			return InvokeRpcEvent(tonumber(netSource.source:sub(5)), ref, {...})
		end
	end,

	__ext = EXT_FUNCREF,

	__pack = function(self, tag)
		local refstr = Citizen.GetFunctionReference(self)
		if refstr then
			return refstr
		else
			error(("Unknown funcref type: %d %s"):format(tag, type(self)), 2)
		end
	end,

	__unpack = function(data, tag)
		local ref = data
		
		-- add a reference
		DuplicateFunctionReference(ref)

		local tbl = {
			__cfx_functionReference = ref,
			__cfx_functionSource = deserializingNetEvent
		}

		if tag == EXT_LOCALFUNCREF then
			tbl.__cfx_functionSource = nil
		end

		tbl = setmetatable(tbl, funcref_mt)

		return tbl
	end,
})

--[[ Also initialize unpackers for local function references --]]
msgpack.extend({
	__ext = EXT_LOCALFUNCREF,
	__pack = funcref_mt.__pack,
	__unpack = funcref_mt.__unpack,
})

msgpack.settype("function", EXT_FUNCREF)

-- exports compatibility
local function getExportEventName(resource, name)
	return string.format('__cfx_export_%s_%s', resource, name)
end

-- callback cache to avoid extra call to serialization / deserialization process at each time getting an export
local exportsCallbackCache = {}

local exportKey = (isDuplicityVersion and 'server_export' or 'export')

do
	local resource = GetCurrentResourceName()

	local numMetaData = GetNumResourceMetadata(resource, exportKey) or 0

	for i = 0, numMetaData-1 do
		local exportName = GetResourceMetadata(resource, exportKey, i)

		AddEventHandler(getExportEventName(resource, exportName), function(setCB)
			-- get the entry from *our* global table and invoke the set callback
			if _G[exportName] then
				setCB(_G[exportName])
			end
		end)
	end
end

-- Remove cache when resource stop to avoid calling unexisting exports
local function lazyEventHandler() -- lazy initializer so we don't add an event we don't need
	AddEventHandler(('on%sResourceStop'):format(isDuplicityVersion and 'Server' or 'Client'), function(resource)
		exportsCallbackCache[resource] = {}
	end)

	lazyEventHandler = function() end
end

-- Helper for newlines in nested error message
local function prefixNewlines(str, prefix)
	str = tostring(str)

	if #str == 0 then
		return str
	end

	return prefix .. str:gsub("\n(.)", "\n" .. prefix .. "%1")
end

-- Handle an export with multiple return values.
local function exportProcessResult(resource, exportName, status, ...)
	if not status then
		local result = tostring(select(1, ...))
		if result:len() > 2048 then
			result = result:sub(1, 1024) .. '\n... [large output partially truncated] ...\n' .. result:sub(-1024)
		end
		error(('\n^5 An error occurred while calling export `%s` in resource `%s`:\n%s\n^5 ---'):format(exportName, resource, prefixNewlines(result, '  ')), 2)
	end
	return ...
end

-- invocation bit
exports = {}

setmetatable(exports, {
	__index = function(t, k)
		local resource = k

		return setmetatable({}, {
			__index = function(t, k)
				lazyEventHandler()

				if not exportsCallbackCache[resource] then
					exportsCallbackCache[resource] = {}
				end

				if not exportsCallbackCache[resource][k] then
					TriggerEvent(getExportEventName(resource, k), function(exportData)
						exportsCallbackCache[resource][k] = exportData
					end)

					if not exportsCallbackCache[resource][k] then
						error('No such export ' .. k .. ' in resource ' .. resource, 2)
					end
				end

				return function(self, ...) -- TAILCALL
					return exportProcessResult(resource, k, pcall(exportsCallbackCache[resource][k], ...))
				end
			end,

			__newindex = function(t, k, v)
				error('cannot set values on an export resource', 2)
			end
		})
	end,

	__newindex = function(t, k, v)
		error('cannot set values on exports', 2)
	end,

	__call = function(t, exportName, func)
		AddEventHandler(getExportEventName(GetCurrentResourceName(), exportName), function(setCB)
			setCB(func)
		end)
	end
})

-- NUI callbacks
if not isDuplicityVersion then
	local origRegisterNuiCallback = RegisterNuiCallback

	local cbHandler

--[==[
	local cbHandler = load([[
		-- Lua 5.4: Create a to-be-closed variable to monitor the NUI callback handle.
		local callback, body, resultCallback = ...

		local hasCallback = false
		local _ <close> = defer(function()
			if not hasCallback then
				local di = debug.getinfo(callback, 'S')
				local name = ('function %s[%d..%d]'):format(di.short_src, di.linedefined, di.lastlinedefined)
				warn(("No NUI callback captured: %s"):format(name))
			end
		end)

		local status, err = pcall(function()
			callback(body, function(...)
				hasCallback = true
				resultCallback(...)
			end)
		end)

		return status, err
	]], '@citizen:/scripting/lua/scheduler.lua#nui')]==]

	if not cbHandler then
		cbHandler = load([[
			local callback, body, resultCallback = ...

			local status, err = pcall(function()
				callback(body, resultCallback)
			end)

			return status, err
		]], '@citizen:/scripting/lua/scheduler.lua#nui')
	end

	-- wrap RegisterNuiCallback to handle errors (and 'missed' callbacks)
	function RegisterNuiCallback(type, callback)
		origRegisterNuiCallback(type, function(body, resultCallback)
			local status, err = cbHandler(callback, body, resultCallback)

			if err then
				Citizen.Trace("error during NUI callback " .. type .. ": " .. tostring(err) .. "\n")
			end
		end)
	end

	-- 'old' function (uses events for compatibility, as people may have relied on this implementation detail)
	function RegisterNUICallback(type, callback)
		RegisterNuiCallbackType(type)

		AddEventHandler('__cfx_nui:' .. type, function(body, resultCallback)
			local status, err = cbHandler(callback, body, resultCallback)

			if err then
				Citizen.Trace("error during NUI callback " .. type .. ": " .. tostring(err) .. "\n")
			end
		end)
	end

	local _sendNuiMessage = SendNuiMessage

	function SendNUIMessage(message)
		_sendNuiMessage(json.encode(message))
	end
end

-- entity helpers
local EXT_ENTITY = 41
local EXT_PLAYER = 42

msgpack.extend_clear(EXT_ENTITY, EXT_PLAYER)

local function NewStateBag(es)
	return setmetatable({}, {
		__index = function(_, s)
			if s == 'set' then
				return function(_, s, v, r)
					local payload = msgpack_pack(v)
					SetStateBagValue(es, s, payload, payload:len(), r)
				end
			end
		
			return GetStateBagValue(es, s)
		end,
		
		__newindex = function(_, s, v)
			local payload = msgpack_pack(v)
			SetStateBagValue(es, s, payload, payload:len(), isDuplicityVersion)
		end
	})
end

GlobalState = NewStateBag('global')

local function GetEntityStateBagId(entityGuid)
	if isDuplicityVersion or NetworkGetEntityIsNetworked(entityGuid) then
		return ('entity:%d'):format(NetworkGetNetworkIdFromEntity(entityGuid))
	else
		EnsureEntityStateBag(entityGuid)
		return ('localEntity:%d'):format(entityGuid)
	end
end

local entityMT
entityMT = {
	__index = function(t, s)
		if s == 'state' then
			local es = GetEntityStateBagId(t.__data)
			
			if isDuplicityVersion then
				EnsureEntityStateBag(t.__data)
			end
		
			return NewStateBag(es)
		end
		
		return nil
	end,
	
	__newindex = function()
		error('Setting values on Entity is not supported at this time.', 2)
	end,
	
	__ext = EXT_ENTITY,
	
	__pack = function(self, t)
		return tostring(NetworkGetNetworkIdFromEntity(self.__data))
	end,
	
	__unpack = function(data, t)
		local ref = NetworkGetEntityFromNetworkId(tonumber(data))
		
		return setmetatable({
			__data = ref
		}, entityMT)
	end
}

msgpack.extend(entityMT)

local playerMT
playerMT = {
	__index = function(t, s)
		if s == 'state' then
			local pid = t.__data
			
			if pid == -1 then
				pid = GetPlayerServerId(PlayerId())
			end
			
			local es = ('player:%d'):format(pid)
		
			return NewStateBag(es)
		end
		
		return nil
	end,
	
	__newindex = function()
		error('Setting values on Player is not supported at this time.', 2)
	end,
	
	__ext = EXT_PLAYER,
	
	__pack = function(self, t)
		return tostring(self.__data)
	end,
	
	__unpack = function(data, t)
		local ref = tonumber(data)
		
		return setmetatable({
			__data = ref
		}, playerMT)
	end
}

msgpack.extend(playerMT)

function Entity(ent)
	if type(ent) == 'number' then
		return setmetatable({
			__data = ent
		}, entityMT)
	end
	
	return ent
end

function Player(ent)
	if type(ent) == 'number' or type(ent) == 'string' then
		return setmetatable({
			__data = tonumber(ent)
		}, playerMT)
	end
	
	return ent
end

if not isDuplicityVersion then
	LocalPlayer = Player(-1)
end
