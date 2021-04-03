local GetGameTimer = GetGameTimer
local _sbs = Citizen.SubmitBoundaryStart
local coresume, costatus = coroutine.resume, coroutine.status
local debug = debug
local coroutine_close = coroutine.close or (function(c) end) -- 5.3 compatibility
local hadThread = false
local curTime = 0
local isDuplicityVersion = IsDuplicityVersion()

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

local function ProfilerEnterScope(scopeName)
	return _in(`PROFILER_ENTER_SCOPE` & 0xFFFFFFFF, scopeName)
end

local function ProfilerExitScope()
	return _in(`PROFILER_EXIT_SCOPE` & 0xFFFFFFFF)
end

local newThreads = {}
local threads = setmetatable({}, {
	-- This circumvents undefined behaviour in "next" (and therefore "pairs")
	__newindex = newThreads,
	-- This is needed for CreateThreadNow to work correctly
	__index = newThreads
})

local boundaryIdx = 1
local runningThread

local function dummyUseBoundary(idx)
	return nil
end

local function getBoundaryFunc(bfn, bid)
	return function(fn, ...)
		local boundary = bid or (boundaryIdx + 1)
		boundaryIdx = boundaryIdx + 1
		
		bfn(boundary, coroutine.running())

		local wrap = function(...)
			dummyUseBoundary(boundary)
			
			local v = table.pack(fn(...))
			return table.unpack(v)
		end
		
		local v = table.pack(wrap(...))
		
		bfn(boundary, nil)
		
		return table.unpack(v)
	end
end

local runWithBoundaryStart = getBoundaryFunc(Citizen.SubmitBoundaryStart)
local runWithBoundaryEnd = getBoundaryFunc(Citizen.SubmitBoundaryEnd)

--[[

	Thread handling

]]
local function resumeThread(coro) -- Internal utility
	if coroutine.status(coro) == "dead" then
		threads[coro] = nil
		coroutine_close(coro)
		return false
	end

	runningThread = coro
	
	local thread = threads[coro]

	if thread then
		if thread.name then
			ProfilerEnterScope(thread.name)
		else
			ProfilerEnterScope('thread')
		end

		_sbs(thread.boundary, coro)
	end
	
	local ok, wakeTimeOrErr = coresume(coro)
	
	if ok then
		thread = threads[coro]
		if thread then
			thread.wakeTime = wakeTimeOrErr or 0
			hadThread = true
		end
	else
		--Citizen.Trace("Error resuming coroutine: " .. debug.traceback(coro, wakeTimeOrErr) .. "\n")
		local fst = FormatStackTrace()
		
		if fst then
			Citizen.Trace("^1SCRIPT ERROR: " .. wakeTimeOrErr .. "^7\n")
			Citizen.Trace(fst)
		end
	end
	
	runningThread = nil
	
	ProfilerExitScope()
	
	-- Return not finished
	return costatus(coro) ~= "dead"
end

function Citizen.CreateThread(threadFunction)
	local bid = boundaryIdx + 1
	boundaryIdx = boundaryIdx + 1

	local tfn = function()
		return runWithBoundaryStart(threadFunction, bid)
	end
	
	local di = debug.getinfo(threadFunction, 'S')
	
	threads[coroutine.create(tfn)] = {
		wakeTime = 0,
		boundary = bid,
		name = ('thread %s[%d..%d]'):format(di.short_src, di.linedefined, di.lastlinedefined)
	}

	hadThread = true
end

function Citizen.Wait(msec)
	coroutine.yield(curTime + msec)
end

-- legacy alias (and to prevent people from calling the game's function)
Wait = Citizen.Wait
CreateThread = Citizen.CreateThread

function Citizen.CreateThreadNow(threadFunction, name)
	local bid = boundaryIdx + 1
	boundaryIdx = boundaryIdx + 1
	curTime = GetGameTimer()
	
	local di = debug.getinfo(threadFunction, 'S')
	name = name or ('thread_now %s[%d..%d]'):format(di.short_src, di.linedefined, di.lastlinedefined)

	local tfn = function()
		return runWithBoundaryStart(threadFunction, bid)
	end

	local coro = coroutine.create(tfn)
	threads[coro] = {
		wakeTime = 0,
		boundary = bid,
		name = name
	}

	return resumeThread(coro)
end

function Citizen.Await(promise)
	local coro = coroutine.running()
	if not coro then
		error("Current execution context is not in the scheduler, you should use CreateThread / SetTimeout or Event system (AddEventHandler) to be able to Await")
	end

	-- Indicates if the promise has already been resolved or rejected
	-- This is a hack since the API does not expose its state
	local isDone = false
	local result, err
	promise = promise:next(function(...)
		isDone = true
		result = {...}
	end,function(error)
		isDone = true
		err = error
	end)

	if not isDone then
		local threadData = threads[coro]
		threads[coro] = nil

		local function reattach()
			threads[coro] = threadData
			resumeThread(coro)
		end

		promise:next(reattach, reattach)
		Citizen.Wait(0)
	end

	if err then
		error(err)
	end

	return table.unpack(result)
end

function Citizen.SetTimeout(msec, callback)
	local bid = boundaryIdx + 1
	boundaryIdx = boundaryIdx + 1

	local tfn = function()
		return runWithBoundaryStart(callback, bid)
	end

	local coro = coroutine.create(tfn)
	threads[coro] = {
		wakeTime = curTime + msec,
		boundary = bid
	}

	hadThread = true
end

SetTimeout = Citizen.SetTimeout

Citizen.SetTickRoutine(function()
	if not hadThread then
		return
	end

	-- flag to skip thread exec if we don't have any
	local thisHadThread = false
	curTime = GetGameTimer()

	for coro, thread in pairs(newThreads) do
		rawset(threads, coro, thread)
		newThreads[coro] = nil

		thisHadThread = true
	end

	for coro, thread in pairs(threads) do
		if curTime >= thread.wakeTime then
			resumeThread(coro)
		end

		thisHadThread = true
	end

	if not thisHadThread then
		hadThread = false
	end
end)

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
	local data = msgpack.unpack(eventPayload)

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

				local di = debug.getinfo(handlerFn)
			
				Citizen.CreateThreadNow(function()
					handler(table.unpack(data))
				end, ('event %s [%s[%d..%d]]'):format(eventName, di.short_src, di.linedefined, di.lastlinedefined))
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
	local n = 1
	
	local frames = {}
	local skip = false
	
	if bs then
		skip = true
	end
	
	repeat
		if ts then
			t = debug.getinfo(ts, n, 'nlfS')
		else
			t = debug.getinfo(n, 'nlfS')
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
					table.insert(frames, {
						file = t.source:sub(2),
						line = t.currentline,
						name = t.name or '[global chunk]'
					})
				end
			end
		
			n = n + 1
		end
	until not t
	
	return msgpack.pack(frames)
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
	if not eventData.key and not eventData.name then
		error('Invalid event data passed to RemoveEventHandler()')
	end

	-- remove the entry
	eventHandlers[eventData.name].handlers[eventData.key] = nil
end

local ignoreNetEvent = {
	'__cfx_internal:commandFallback'
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
	local payload = msgpack.pack({...})

	return runWithBoundaryEnd(function()
		return TriggerEventInternal(eventName, payload, payload:len())
	end)
end

if isDuplicityVersion then
	function TriggerClientEvent(eventName, playerId, ...)
		local payload = msgpack.pack({...})

		return TriggerClientEventInternal(eventName, playerId, payload, payload:len())
	end
	
	function TriggerLatentClientEvent(eventName, playerId, bps, ...)
		local payload = msgpack.pack({...})

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
			table.insert(t, GetPlayerIdentifier(player, i))
		end

		return t
	end

	function GetPlayers()
		local num = GetNumPlayerIndices()
		local t = {}

		for i = 0, num - 1 do
			table.insert(t, GetPlayerFromIndex(i))
		end

		return t
	end

	local httpDispatch = {}
	AddEventHandler('__cfx_internal:httpResponse', function(token, status, body, headers)
		if httpDispatch[token] then
			local userCallback = httpDispatch[token]
			httpDispatch[token] = nil
			userCallback(status, body, headers)
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

		local d = json.encode(t)
		local id = PerformHttpRequestInternal(d, d:len())

		httpDispatch[id] = cb
	end
else
	function TriggerServerEvent(eventName, ...)
		local payload = msgpack.pack({...})

		return TriggerServerEventInternal(eventName, payload, payload:len())
	end
	
	function TriggerLatentServerEvent(eventName, bps, ...)
		local payload = msgpack.pack({...})

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

	return '^1SCRIPT ERROR: ' .. err .. "^7\n" .. fst
end

Citizen.SetCallRefRoutine(function(refId, argsSerialized)
	local refPtr = funcRefs[refId]

	if not refPtr then
		Citizen.Trace('Invalid ref call attempt: ' .. refId .. "\n")

		return msgpack.pack(nil)
	end
	
	local ref = refPtr.func

	local err
	local retvals
	local cb = {}
	
	local di = debug.getinfo(ref)

	local waited = Citizen.CreateThreadNow(function()
		local status, result, error = xpcall(function()
			retvals = { ref(table.unpack(msgpack.unpack(argsSerialized))) }
		end, doStackFormat)

		if not status then
			err = result or ''
		end

		if cb.cb then
			cb.cb(retvals or false, err)
		end
	end, ('ref call [%s[%d..%d]]'):format(di.short_src, di.linedefined, di.lastlinedefined))

	if not waited then
		if err then
			--error(err)
			if err ~= '' then
				Citizen.Trace(err)
			end
			
			return msgpack.pack(nil)
		end

		return msgpack.pack(retvals)
	else
		return msgpack.pack({{
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
			local payload = Citizen.InvokeFunctionReference(refId, msgpack.pack(args))

			if #payload == 0 then
				returnEvent(false, 'err')
				return
			end

			local rvs = msgpack.unpack(payload)

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
	if not coroutine.running() then
		error('RPC delegates can only be invoked from a thread.')
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
		error('Cannot index a funcref')
	end,

	__newindex = function(t, k, v)
		error('Cannot set indexes on a funcref')
	end,

	__call = function(t, ...)
		local netSource = rawget(t, '__cfx_functionSource')
		local ref = rawget(t, '__cfx_functionReference')

		if not netSource then
			local args = msgpack.pack({...})

			-- as Lua doesn't allow directly getting lengths from a data buffer, and _s will zero-terminate, we have a wrapper in the game itself
			local rv = runWithBoundaryEnd(function()
				return Citizen.InvokeFunctionReference(ref, args)
			end)
			local rvs = msgpack.unpack(rv)

			-- handle async retvals from refs
			if rvs and type(rvs[1]) == 'table' and rawget(rvs[1], '__cfx_async_retval') and coroutine.running() then
				local p = promise.new()

				rvs[1].__cfx_async_retval(function(r, e)
					if r then
						p:resolve(r)
					elseif e then
						p:reject(e)
					end
				end)

				return table.unpack(Citizen.Await(p))
			end

			return table.unpack(rvs)
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
			error(("Unknown funcref type: %d %s"):format(tag, type(self)))
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
						error('No such export ' .. k .. ' in resource ' .. resource)
					end
				end

				return function(self, ...)
					local status, result = pcall(exportsCallbackCache[resource][k], ...)

					if not status then
						error('An error happened while calling export ' .. k .. ' of resource ' .. resource .. ' (' .. result .. '), see above for details')
					end

					return result
				end
			end,

			__newindex = function(t, k, v)
				error('cannot set values on an export resource')
			end
		})
	end,

	__newindex = function(t, k, v)
		error('cannot set values on exports')
	end,

	__call = function(t, exportName, func)
		AddEventHandler(getExportEventName(GetCurrentResourceName(), exportName), function(setCB)
			setCB(func)
		end)
	end
})

-- NUI callbacks
if not isDuplicityVersion then
	function RegisterNUICallback(type, callback)
		RegisterNuiCallbackType(type)

		AddEventHandler('__cfx_nui:' .. type, function(body, resultCallback)
			local status, err = pcall(function()
				callback(body, resultCallback)
			end)

			if err then
				Citizen.Trace("error during NUI callback " .. type .. ": " .. err .. "\n")
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
					local payload = msgpack.pack(v)
					SetStateBagValue(es, s, payload, payload:len(), r)
				end
			end
		
			return GetStateBagValue(es, s)
		end,
		
		__newindex = function(_, s, v)
			local payload = msgpack.pack(v)
			SetStateBagValue(es, s, payload, payload:len(), isDuplicityVersion)
		end
	})
end

GlobalState = NewStateBag('global')

local entityTM = {
	__index = function(t, s)
		if s == 'state' then
			local es = ('entity:%d'):format(NetworkGetNetworkIdFromEntity(t.__data))
			
			if isDuplicityVersion then
				EnsureEntityStateBag(t.__data)
			end
		
			return NewStateBag(es)
		end
		
		return nil
	end,
	
	__newindex = function()
		error('Not allowed at this time.')
	end,
	
	__ext = EXT_ENTITY,
	
	__pack = function(self, t)
		return tostring(NetworkGetNetworkIdFromEntity(self.__data))
	end,
	
	__unpack = function(data, t)
		local ref = NetworkGetEntityFromNetworkId(tonumber(data))
		
		return setmetatable({
			__data = ref
		}, entityTM)
	end
}

msgpack.extend(entityTM)

local playerTM = {
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
		error('Not allowed at this time.')
	end,
	
	__ext = EXT_PLAYER,
	
	__pack = function(self, t)
		return tostring(self.__data)
	end,
	
	__unpack = function(data, t)
		local ref = tonumber(data)
		
		return setmetatable({
			__data = ref
		}, playerTM)
	end
}

msgpack.extend(playerTM)

function Entity(ent)
	if type(ent) == 'number' then
		return setmetatable({
			__data = ent
		}, entityTM)
	end
	
	return ent
end

function Player(ent)
	if type(ent) == 'number' or type(ent) == 'string' then
		return setmetatable({
			__data = tonumber(ent)
		}, playerTM)
	end
	
	return ent
end

if not isDuplicityVersion then
	LocalPlayer = Player(-1)
end
