--[[
    Anticheat System - Event Detection
    Validates server events and implements rate limiting
]]

EventDetection = {}

local playerEventCounts = {}
local blockedEventAttempts = {}

-- Initialize player tracking
function EventDetection.initPlayer(playerId)
    playerEventCounts[playerId] = {
        counts = {},
        lastReset = GetGameTimer(),
        violations = 0
    }
end

-- Clean up player data
function EventDetection.cleanupPlayer(playerId)
    playerEventCounts[playerId] = nil
end

-- Check if event is blocked
function EventDetection.isBlocked(eventName)
    for _, blocked in ipairs(Config.Detections.Events.BlockedEvents) do
        if eventName == blocked or string.match(eventName, blocked) then
            return true
        end
    end
    return false
end

-- Track event for rate limiting
function EventDetection.trackEvent(playerId, eventName)
    if not Config.Detections.Events.Enabled then return true end
    
    local data = playerEventCounts[playerId]
    if not data then
        EventDetection.initPlayer(playerId)
        data = playerEventCounts[playerId]
    end
    
    local currentTime = GetGameTimer()
    
    -- Reset counts every second
    if currentTime - data.lastReset > 1000 then
        data.counts = {}
        data.lastReset = currentTime
    end
    
    -- Increment count for this second
    data.counts[eventName] = (data.counts[eventName] or 0) + 1
    
    -- Calculate total events this second
    local totalEvents = 0
    for _, count in pairs(data.counts) do
        totalEvents = totalEvents + count
    end
    
    -- Check rate limit
    if totalEvents > Config.Detections.Events.RateLimitPerSecond then
        EventDetection.onViolation(playerId, "rate_limit", {
            events_per_second = totalEvents,
            limit = Config.Detections.Events.RateLimitPerSecond,
            last_event = eventName
        })
        return false
    end
    
    return true
end

-- Validate incoming event
function EventDetection.validate(playerId, eventName, ...)
    if not Config.Detections.Events.Enabled then return true end
    
    local playerName = GetPlayerName(playerId)
    local identifiers = GetPlayerIdentifiers(playerId)
    
    -- Check if event is blocked
    if EventDetection.isBlocked(eventName) then
        EventDetection.onViolation(playerId, "blocked_event", {
            event_name = eventName
        })
        return false
    end
    
    -- Rate limit check
    if not EventDetection.trackEvent(playerId, eventName) then
        return false
    end
    
    return true
end

-- Handle violation
function EventDetection.onViolation(playerId, violationType, details)
    local data = playerEventCounts[playerId]
    if data then
        data.violations = data.violations + 1
    end
    
    local playerName = GetPlayerName(playerId)
    local identifiers = GetPlayerIdentifiers(playerId)
    
    Logger.detection("event_" .. violationType, 
        API.getPrimaryIdentifier(identifiers), 
        playerName, 
        details
    )
    
    -- Check if threshold reached
    local violations = data and data.violations or 1
    if violations >= Config.Detections.Events.GraceViolations then
        TriggerEvent("anticheat:sanction", playerId, Config.Detections.Events.Action, 
            "Event violation: " .. violationType, details)
        if data then data.violations = 0 end
    end
    
    -- Report to API
    API.reportDetection(playerId, identifiers, "event_" .. violationType, "medium", details)
end

-- Create protected event wrapper
function EventDetection.protectEvent(eventName, handler)
    RegisterNetEvent(eventName)
    AddEventHandler(eventName, function(...)
        local playerId = source
        if playerId and playerId > 0 then
            if not EventDetection.validate(playerId, eventName, ...) then
                Logger.warn("events", "blocked_event_call", {
                    event = eventName,
                    player = GetPlayerName(playerId)
                })
                return
            end
        end
        handler(...)
    end)
end

-- Monitor all server events (if possible via event handlers)
-- This provides a centralized point for event validation
RegisterNetEvent("anticheat:validateEvent")
AddEventHandler("anticheat:validateEvent", function(eventName)
    local playerId = source
    EventDetection.validate(playerId, eventName)
end)
