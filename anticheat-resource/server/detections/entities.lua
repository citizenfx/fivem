--[[
    Anticheat System - Entity Detection
    Detects unauthorized entity spawning
]]

EntityDetection = {}

local playerEntityCounts = {}

-- Initialize player tracking
function EntityDetection.initPlayer(playerId)
    playerEntityCounts[playerId] = 0
end

-- Clean up player data
function EntityDetection.cleanupPlayer(playerId)
    playerEntityCounts[playerId] = nil
end

-- Check entity creation
function EntityDetection.onEntityCreated(entity, entityType)
    if not Config.Detections.Entities.Enabled then return end
    
    -- Get the entity owner
    local owner = NetworkGetEntityOwner(entity)
    if not owner or owner == 0 then return end
    
    local playerId = owner
    local playerName = GetPlayerName(playerId)
    local identifiers = GetPlayerIdentifiers(playerId)
    
    -- Check if player is allowed to spawn this type
    local allowed = false
    for _, allowedType in ipairs(Config.Detections.Entities.AllowedClientSpawns) do
        if entityType == allowedType then
            allowed = true
            break
        end
    end
    
    -- Get entity model
    local model = GetEntityModel(entity)
    
    -- Check for blacklisted models
    for _, blacklistedModel in ipairs(Config.Detections.Entities.BlacklistedModels) do
        if model == blacklistedModel then
            EntityDetection.onViolation(playerId, "blacklisted_model", {
                model_hash = model,
                entity_type = entityType
            })
            -- Delete the entity
            if DoesEntityExist(entity) then
                DeleteEntity(entity)
            end
            return
        end
    end
    
    -- Track entity count per player
    if playerEntityCounts[playerId] then
        playerEntityCounts[playerId] = playerEntityCounts[playerId] + 1
        
        if playerEntityCounts[playerId] > Config.Detections.Entities.MaxEntitiesPerPlayer then
            EntityDetection.onViolation(playerId, "entity_spam", {
                entity_count = playerEntityCounts[playerId],
                max_allowed = Config.Detections.Entities.MaxEntitiesPerPlayer
            })
        end
    end
    
    -- If not allowed and lockdown is enabled
    if Config.Detections.Entities.EntityLockdown and not allowed then
        EntityDetection.onViolation(playerId, "unauthorized_spawn", {
            entity_type = entityType,
            model_hash = model
        })
        -- Delete unauthorized entity
        if DoesEntityExist(entity) then
            DeleteEntity(entity)
        end
    end
end

-- Handle violation
function EntityDetection.onViolation(playerId, violationType, details)
    local playerName = GetPlayerName(playerId)
    local identifiers = GetPlayerIdentifiers(playerId)
    
    Logger.detection("entity_" .. violationType, 
        API.getPrimaryIdentifier(identifiers), 
        playerName, 
        details
    )
    
    -- Entity violations are usually more severe
    TriggerEvent("anticheat:sanction", playerId, Config.Detections.Entities.Action, 
        "Entity violation: " .. violationType, details)
    
    -- Report to API
    API.reportDetection(playerId, identifiers, "entity_" .. violationType, "high", details)
end

-- Reset entity count (called periodically)
function EntityDetection.resetCounts()
    for playerId, _ in pairs(playerEntityCounts) do
        playerEntityCounts[playerId] = 0
    end
end

-- Set up entity creation handler
AddEventHandler("entityCreated", function(entity)
    local entityType = GetEntityType(entity)
    local typeNames = {[1] = "ped", [2] = "vehicle", [3] = "object"}
    EntityDetection.onEntityCreated(entity, typeNames[entityType] or "unknown")
end)

-- Reset counts periodically
CreateThread(function()
    while true do
        Wait(60000) -- Every minute
        EntityDetection.resetCounts()
    end
end)
