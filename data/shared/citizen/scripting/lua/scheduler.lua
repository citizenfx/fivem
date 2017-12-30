local threads = {}
local curThread
local curThreadIndex

function Citizen.CreateThread(threadFunction)
	table.insert(threads, {
		coroutine = coroutine.create(threadFunction),
		wakeTime = 0
	})
end

function Citizen.Wait(msec)
	curThread.wakeTime = GetGameTimer() + msec

	coroutine.yield()
end

-- legacy alias (and to prevent people from calling the game's function)
Wait = Citizen.Wait
CreateThread = Citizen.CreateThread

function Citizen.CreateThreadNow(threadFunction)
	local coro = coroutine.create(threadFunction)

	local t = {
		coroutine = coro,
		wakeTime = 0
	}

	-- add new thread and save old thread
	local oldThread = curThread
	curThread = t

	local result, err = coroutine.resume(coro)

	local resumedThread = curThread
	-- restore last thread
	curThread = oldThread

	if err then
		error('Failed to execute thread: ' .. debug.traceback(coro, err))
	end

	if resumedThread and coroutine.status(coro) ~= 'dead' then
		table.insert(threads, t)
	end

	return coroutine.status(coro) ~= 'dead'
end

function Citizen.Await(promise)
	if not curThread then
		error("Current execution context is not in the scheduler, you should use CreateThread / SetTimeout or Event system (AddEventHandler) to be able to Await")
	end

	-- Remove current thread from the pool (avoid resume from the loop)
	if curThreadIndex then
		table.remove(threads, curThreadIndex)
	end

	curThreadIndex = nil
	local resumableThread = curThread

	promise:next(function (result)
		-- Reattach thread
		table.insert(threads, resumableThread)

		curThread = resumableThread
		curThreadIndex = #threads

		local result, err = coroutine.resume(resumableThread.coroutine, result)

		if err then
			error('Failed to resume thread: ' .. debug.traceback(resumableThread.coroutine, err))
		end

		return result
	end, function (err)
		if err then
			Citizen.Trace('Await failure: ' .. debug.traceback(resumableThread.coroutine, err, 2))
		end
	end)

	curThread = nil
	return coroutine.yield()
end

-- SetTimeout
local timeouts = {}

function Citizen.SetTimeout(msec, callback)
	table.insert(threads, {
		coroutine = coroutine.create(callback),
		wakeTime = GetGameTimer() + msec
	})
end

SetTimeout = Citizen.SetTimeout

Citizen.SetTickRoutine(function()
	local curTime = GetGameTimer()

	for i = #threads, 1, -1 do
		local thread = threads[i]

		if curTime >= thread.wakeTime then
			curThread = thread
			curThreadIndex = i

			local status = coroutine.status(thread.coroutine)

			if status == 'dead' then
				table.remove(threads, i)
			else
				local result, err = coroutine.resume(thread.coroutine)

				if not result then
					Citizen.Trace("Error resuming coroutine: " .. debug.traceback(thread.coroutine, err) .. "\n")

					table.remove(threads, i)
				end
			end
		end
	end

	curThread = nil
	curThreadIndex = nil
end)

local alwaysSafeEvents = {
	["playerDropped"] = true,
	["playerConnecting"] = true
}

local eventHandlers = {}
local deserializingNetEvent = false

Citizen.SetEventRoutine(function(eventName, eventPayload, eventSource)
	-- set the event source
	local lastSource = _G.source
	_G.source = eventSource

	-- try finding an event handler for the event
	local eventHandlerEntry = eventHandlers[eventName]

	if eventHandlerEntry and eventHandlerEntry.handlers then
		-- if this is a net event and we don't allow this event to be triggered from the network, return
		if eventSource:sub(1, 3) == 'net' then
			if not eventHandlerEntry.safeForNet and not alwaysSafeEvents[eventName] then
				Citizen.Trace('event ' .. eventName .. " was not safe for net\n")

				return
			end

			deserializingNetEvent = { source = eventSource }
			_G.source = tonumber(eventSource:sub(5))
		end

		-- if we found one, deserialize the data structure
		local data = msgpack.unpack(eventPayload)

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
				Citizen.CreateThreadNow(function()
					handler(table.unpack(data))
				end)
			end
		end
	end

	_G.source = lastSource
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

function RegisterNetEvent(eventName)
	local tableEntry = eventHandlers[eventName]

	if not tableEntry then
		tableEntry = { }

		eventHandlers[eventName] = tableEntry
	end

	tableEntry.safeForNet = true
end

function TriggerEvent(eventName, ...)
	local payload = msgpack.pack({...})

	return TriggerEventInternal(eventName, payload, payload:len())
end

if IsDuplicityVersion() then
	function TriggerClientEvent(eventName, playerId, ...)
		local payload = msgpack.pack({...})

		return TriggerClientEventInternal(eventName, playerId, payload, payload:len())
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

	function PerformHttpRequest(url, cb, method, data, headers)
		local t = {
			url = url,
			method = method or 'GET',
			data = data or '',
			headers = headers or {}
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
end

local funcRefs = {}
local funcRefIdx = 0

local function MakeFunctionReference(func)
	local thisIdx = funcRefIdx

	funcRefs[thisIdx] = func

	funcRefIdx = funcRefIdx + 1

	return Citizen.CanonicalizeRef(thisIdx)
end

function Citizen.GetFunctionReference(func)
	if type(func) == 'function' then
		return MakeFunctionReference(func)
	elseif type(func) == 'table' and rawget(table, '__cfx_functionReference') then
		return DuplicateFunctionReference(rawget(table, '__cfx_functionReference'))
	end

	return nil
end

Citizen.SetCallRefRoutine(function(refId, argsSerialized)
	local ref = funcRefs[refId]

	if not ref then
		Citizen.Trace('Invalid ref call attempt: ' .. refId .. "\n")

		return msgpack.pack({})
	end

	local err
	local retvals
	local cb = {}

	local waited = Citizen.CreateThreadNow(function()
		local status, result, error = xpcall(function()
			retvals = { ref(table.unpack(msgpack.unpack(argsSerialized))) }
		end, debug.traceback)

		if not status then
			err = result
		end

		if cb.cb then
			cb.cb(retvals or false, err)
		end
	end)

	if not waited then
		if err then
			error(err)
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
		local thisIdx = funcRefIdx
		funcRefs[thisIdx] = ref

		funcRefIdx = funcRefIdx + 1

		return thisIdx
	end

	return -1
end)

Citizen.SetDeleteRefRoutine(function(refId)
	funcRefs[refId] = nil
end)

local EXT_FUNCREF = 10
local EXT_LOCALFUNCREF = 11

msgpack.packers['funcref'] = function(buffer, ref)
	msgpack.packers['ext'](buffer, EXT_FUNCREF, ref)
end

msgpack.packers['table'] = function(buffer, table)
	if rawget(table, '__cfx_functionReference') then
		-- pack as function reference
		msgpack.packers['funcref'](buffer, DuplicateFunctionReference(rawget(table, '__cfx_functionReference')))
	else
		msgpack.packers['_table'](buffer, table)
	end
end

msgpack.packers['function'] = function(buffer, func)
	msgpack.packers['funcref'](buffer, MakeFunctionReference(func))
end

-- RPC REQUEST HANDLER
local InvokeRpcEvent

if GetCurrentResourceName() == 'sessionmanager' then
	local rpcEvName = ('__cfx_rpcReq')

	RegisterNetEvent(rpcEvName)

	AddEventHandler(rpcEvName, function(retEvent, retId, refId, args)
		local source = source

		local eventTriggerFn = TriggerServerEvent
		
		if IsDuplicityVersion() then
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

if IsDuplicityVersion() then
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

-- RPC INVOCATION
InvokeRpcEvent = function(source, ref, args)
	if not curThread then
		error('RPC delegates can only be invoked from a thread.')
	end

	local src = source

	local eventTriggerFn = TriggerServerEvent

	if IsDuplicityVersion() then
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

local funcref_mt = {
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
			local rv = Citizen.InvokeFunctionReference(ref, args)
			local rvs = msgpack.unpack(rv)

			-- handle async retvals from refs
			if rvs and type(rvs[1]) == 'table' and rawget(rvs[1], '__cfx_async_retval') and curThread then
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
	end
}

msgpack.build_ext = function(tag, data)
	if tag == EXT_FUNCREF or tag == EXT_LOCALFUNCREF then
		local ref = data

		local tbl = {
			__cfx_functionReference = ref,
			__cfx_functionSource = deserializingNetEvent
		}

		if tag == EXT_LOCALFUNCREF then
			tbl.__cfx_functionSource = nil
		end

		tbl = setmetatable(tbl, funcref_mt)

		return tbl
	end
end

-- exports compatibility
local function getExportEventName(resource, name)
	return string.format('__cfx_export_%s_%s', resource, name)
end

-- callback cache to avoid extra call to serialization / deserialization process at each time getting an export
local exportsCallbackCache = {}

local exportKey = (IsDuplicityVersion() and 'server_export' or 'export')

AddEventHandler(('on%sResourceStart'):format(IsDuplicityVersion() and 'Server' or 'Client'), function(resource)
	if resource == GetCurrentResourceName() then
		local numMetaData = GetNumResourceMetadata(resource, exportKey) or 0

		for i = 0, numMetaData-1 do
			local exportName = GetResourceMetadata(resource, exportKey, i)

			AddEventHandler(getExportEventName(resource, exportName), function(setCB)
				-- get the entry from *our* global table and invoke the set callback
				setCB(_G[exportName])
			end)
		end
	end
end)

-- Remove cache when resource stop to avoid calling unexisting exports
AddEventHandler(('on%sResourceStop'):format(IsDuplicityVersion() and 'Server' or 'Client'), function(resource)
	exportsCallbackCache[resource] = {}
end)

-- invocation bit
exports = {}

setmetatable(exports, {
	__index = function(t, k)
		local resource = k

		return setmetatable({}, {
			__index = function(t, k)
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
	end
})

-- NUI callbacks
if not IsDuplicityVersion() then
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
