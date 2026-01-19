--[[
    Anticheat System - Health/Godmode Detection
    Detects invalid health values and godmode
]]

HealthDetection = {}

local playerData = {}

-- Initialize player tracking
function HealthDetection.initPlayer(playerId)
    playerData[playerId] = {
        lastHealth = 200,
        lastArmor = 0,
        damageTaken = false,
        lastDamageCheck = GetGameTimer(),
        violations = 0
    }
end

-- Clean up player data
function HealthDetection.cleanupPlayer(playerId)
    playerData[playerId] = nil
end

-- Check player health values
function HealthDetection.check(playerId, health, armor)
    if not Config.Detections.Health.Enabled then return end
    
    local data = playerData[playerId]
    if not data then
        HealthDetection.initPlayer(playerId)
        data = playerData[playerId]
    end
    
    local playerName = GetPlayerName(playerId)
    local identifiers = GetPlayerIdentifiers(playerId)
    local violation = nil
    
    -- Check for invalid health values
    if health > Config.Detections.Health.MaxHealth then
        violation = {
            type = "invalid_health",
            details = {
                current_health = health,
                max_allowed = Config.Detections.Health.MaxHealth
            }
        }
    elseif health < Config.Detections.Health.MinHealth then
        violation = {
            type = "negative_health",
            details = {
                current_health = health,
                min_allowed = Config.Detections.Health.MinHealth
            }
        }
    end
    
    -- Check for invalid armor values
    if armor > Config.Detections.Health.MaxArmor then
        violation = {
            type = "invalid_armor",
            details = {
                current_armor = armor,
                max_allowed = Config.Detections.Health.MaxArmor
            }
        }
    end
    
    -- Handle violation
    if violation then
        HealthDetection.onViolation(playerId, violation.type, violation.details)
    end
    
    -- Track health changes for godmode detection
    if health < data.lastHealth then
        data.damageTaken = true
    end
    
    data.lastHealth = health
    data.lastArmor = armor
end

-- Check for godmode (called periodically)
function HealthDetection.checkGodmode(playerId)
    if not Config.Detections.Health.Enabled then return end
    
    local data = playerData[playerId]
    if not data then return end
    
    local currentTime = GetGameTimer()
    local timeSinceCheck = currentTime - data.lastDamageCheck
    
    -- Only check if enough time has passed
    if timeSinceCheck < Config.Detections.Health.GodmodeCheckInterval then
        return
    end
    
    -- If player has been in combat but took no damage, suspicious
    -- This would need integration with damage events
    -- For now, we rely on client-side reporting
    
    data.lastDamageCheck = currentTime
    data.damageTaken = false
end

-- Handle violation
function HealthDetection.onViolation(playerId, violationType, details)
    local data = playerData[playerId]
    if not data then return end
    
    data.violations = data.violations + 1
    local playerName = GetPlayerName(playerId)
    local identifiers = GetPlayerIdentifiers(playerId)
    
    Logger.detection("health_" .. violationType, 
        API.getPrimaryIdentifier(identifiers), 
        playerName, 
        details
    )
    
    -- Check if threshold reached
    if data.violations >= Config.Detections.Health.GraceViolations then
        TriggerEvent("anticheat:sanction", playerId, Config.Detections.Health.Action, 
            "Health violation: " .. violationType, details)
        data.violations = 0
    end
    
    -- Report to API
    API.reportDetection(playerId, identifiers, "health_" .. violationType, "high", details)
end

-- Get player data (for debugging)
function HealthDetection.getPlayerData(playerId)
    return playerData[playerId]
end
