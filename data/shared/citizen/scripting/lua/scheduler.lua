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

Citizen.SetEventRoutine(function(eventName, eventPayload, eventSource)
	-- set the event source
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

			_G.source = tonumber(eventSource:sub(5))
		end

		-- if we found one, deserialize the data structure
		local data = msgpack.unpack(eventPayload)

		-- return an empty table if the data is nil
		if not data then
			data = {}
		end

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

	return msgpack.pack({ ref(table.unpack(msgpack.unpack(argsSerialized))) })
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
		local ref = rawget(t, '__cfx_functionReference')
		local args = msgpack.pack({...})

		-- as Lua doesn't allow directly getting lengths from a data buffer, and _s will zero-terminate, we have a wrapper in the game itself
		local rv = Citizen.InvokeFunctionReference(ref, args)

		return table.unpack(msgpack.unpack(rv))
	end
}

msgpack.build_ext = function(tag, data)
	if tag == EXT_FUNCREF then
		local ref = data

		local tbl = {
			__cfx_functionReference = ref
		}

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
