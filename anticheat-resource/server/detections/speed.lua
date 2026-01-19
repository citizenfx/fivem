--[[
    Anticheat System - Speed/Teleportation Detection
    Detects impossible movement speeds and teleportation
]]

SpeedDetection = {}

local playerData = {}

-- Initialize player tracking
function SpeedDetection.initPlayer(playerId)
    playerData[playerId] = {
        lastPosition = nil,
        lastTime = GetGameTimer(),
        violations = 0,
        inVehicle = false
    }
end

-- Clean up player data
function SpeedDetection.cleanupPlayer(playerId)
    playerData[playerId] = nil
end

-- Check player speed based on reported position
function SpeedDetection.check(playerId, position, inVehicle)
    if not Config.Detections.Speed.Enabled then return end
    
    local data = playerData[playerId]
    if not data then
        SpeedDetection.initPlayer(playerId)
        data = playerData[playerId]
    end
    
    local currentTime = GetGameTimer()
    data.inVehicle = inVehicle
    
    -- First position, just record it
    if not data.lastPosition then
        data.lastPosition = position
        data.lastTime = currentTime
        return
    end
    
    -- Calculate distance and time delta
    local dx = position.x - data.lastPosition.x
    local dy = position.y - data.lastPosition.y
    local dz = position.z - data.lastPosition.z
    local distance = math.sqrt(dx*dx + dy*dy + dz*dz)
    local timeDelta = (currentTime - data.lastTime) / 1000 -- Convert to seconds
    
    -- Avoid division by zero
    if timeDelta < 0.1 then
        return
    end
    
    local speed = distance / timeDelta
    
    -- Determine max allowed speed
    local maxSpeed = inVehicle 
        and Config.Detections.Speed.MaxVehicleSpeed 
        or Config.Detections.Speed.MaxFootSpeed
    
    -- Check for teleportation
    if distance > Config.Detections.Speed.TeleportThreshold then
        SpeedDetection.onViolation(playerId, "teleport", {
            distance = distance,
            threshold = Config.Detections.Speed.TeleportThreshold,
            from = data.lastPosition,
            to = position
        })
    -- Check for speed hack
    elseif speed > maxSpeed then
        SpeedDetection.onViolation(playerId, "speed", {
            speed = speed,
            max_allowed = maxSpeed,
            in_vehicle = inVehicle,
            position = position
        })
    else
        -- Reset violations on clean check
        if data.violations > 0 then
            data.violations = data.violations - 1
        end
    end
    
    -- Update tracking data
    data.lastPosition = position
    data.lastTime = currentTime
end

-- Handle violation
function SpeedDetection.onViolation(playerId, violationType, details)
    local data = playerData[playerId]
    if not data then return end
    
    data.violations = data.violations + 1
    local playerName = GetPlayerName(playerId)
    local identifiers = GetPlayerIdentifiers(playerId)
    
    Logger.detection("speed_" .. violationType, 
        API.getPrimaryIdentifier(identifiers), 
        playerName, 
        details
    )
    
    -- Check if threshold reached
    if data.violations >= Config.Detections.Speed.GraceViolations then
        TriggerEvent("anticheat:sanction", playerId, Config.Detections.Speed.Action, 
            "Speed/Teleportation violation: " .. violationType, details)
        data.violations = 0
    end
    
    -- Report to API
    API.reportDetection(playerId, identifiers, "speed_" .. violationType, "medium", details)
end

-- Get player data (for debugging)
function SpeedDetection.getPlayerData(playerId)
    return playerData[playerId]
end
