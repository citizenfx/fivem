--[[
    Anticheat System - Main Server Script
    Core coordination and sanction handling
]]

local playerStates = {}
local bannedPlayers = {} -- In-memory cache, should sync with API/database

-- ============================================================================
-- Initialization
-- ============================================================================

AddEventHandler("onResourceStart", function(resourceName)
    if GetCurrentResourceName() ~= resourceName then return end
    
    Logger.info("main", "starting", {
        version = GetResourceMetadata(GetCurrentResourceName(), "version", 0),
        debug = Config.Debug
    })
    
    -- Initialize API connection
    API.init()
    
    Logger.info("main", "initialized", {
        detections_enabled = {
            speed = Config.Detections.Speed.Enabled,
            health = Config.Detections.Health.Enabled,
            weapons = Config.Detections.Weapons.Enabled,
            entities = Config.Detections.Entities.Enabled,
            events = Config.Detections.Events.Enabled,
            resources = Config.Detections.Resources.Enabled
        }
    })
end)

-- ============================================================================
-- Player Connection Handling
-- ============================================================================

AddEventHandler("playerConnecting", function(name, setKickReason, deferrals)
    local playerId = source
    local identifiers = GetPlayerIdentifiers(playerId)
    local license = API.getPrimaryIdentifier(identifiers)
    
    deferrals.defer()
    deferrals.update("Checking anticheat status...")
    
    -- Check if player is whitelisted
    if isWhitelisted(playerId, identifiers) then
        Wait(100)
        deferrals.done()
        return
    end
    
    -- Check local ban cache
    if bannedPlayers[license] then
        local ban = bannedPlayers[license]
        if ban.expires_at == 0 or ban.expires_at > os.time() then
            deferrals.done("You are banned. Reason: " .. (ban.reason or "Unknown") .. 
                " | Expires: " .. (ban.expires_at == 0 and "Never" or os.date("%Y-%m-%d %H:%M", ban.expires_at)))
            return
        end
    end
    
    -- Check with API if connected
    if API.isConnected() then
        API.getPlayerInfo(license, function(success, data)
            if success and data and data.banned then
                deferrals.done("You are banned. Reason: " .. (data.ban_reason or "Unknown"))
                return
            end
            deferrals.done()
        end)
        
        -- Timeout fallback
        Wait(3000)
        if deferrals then
            deferrals.done()
        end
    else
        Wait(100)
        deferrals.done()
    end
end)

AddEventHandler("playerJoining", function(oldId)
    local playerId = source
    local identifiers = GetPlayerIdentifiers(playerId)
    
    -- Initialize detection modules for this player
    SpeedDetection.initPlayer(playerId)
    HealthDetection.initPlayer(playerId)
    WeaponDetection.initPlayer(playerId)
    EntityDetection.initPlayer(playerId)
    EventDetection.initPlayer(playerId)
    
    -- Store player state
    playerStates[playerId] = {
        identifiers = identifiers,
        joinTime = os.time(),
        totalDetections = 0
    }
    
    -- Notify API
    API.playerConnected(playerId, identifiers)
    
    Logger.info("main", "player_joined", {
        name = GetPlayerName(playerId)
    }, API.getPrimaryIdentifier(identifiers), GetPlayerName(playerId))
end)

AddEventHandler("playerDropped", function(reason)
    local playerId = source
    local state = playerStates[playerId]
    local identifiers = state and state.identifiers or {}
    
    -- Cleanup detection modules
    SpeedDetection.cleanupPlayer(playerId)
    HealthDetection.cleanupPlayer(playerId)
    WeaponDetection.cleanupPlayer(playerId)
    EntityDetection.cleanupPlayer(playerId)
    EventDetection.cleanupPlayer(playerId)
    
    -- Notify API
    API.playerDisconnected(playerId, identifiers, reason)
    
    Logger.info("main", "player_dropped", {
        reason = reason
    }, API.getPrimaryIdentifier(identifiers), GetPlayerName(playerId))
    
    -- Cleanup state
    playerStates[playerId] = nil
end)

-- ============================================================================
-- Client State Reporting Handler
-- ============================================================================

RegisterNetEvent("anticheat:reportState")
AddEventHandler("anticheat:reportState", function(state)
    local playerId = source
    
    if not state or type(state) ~= "table" then return end
    
    -- Validate event rate
    if not EventDetection.validate(playerId, "anticheat:reportState") then
        return
    end
    
    -- Process position/speed
    if state.position then
        SpeedDetection.check(playerId, state.position, state.inVehicle)
    end
    
    -- Process health
    if state.health ~= nil then
        HealthDetection.check(playerId, state.health, state.armor or 0)
    end
    
    -- Process weapons
    if state.weapon then
        WeaponDetection.check(playerId, state.weapon, state.ammo or 0)
    end
end)

-- Weapon fire event
RegisterNetEvent("anticheat:weaponFired")
AddEventHandler("anticheat:weaponFired", function(weaponHash)
    local playerId = source
    WeaponDetection.onWeaponFire(playerId, weaponHash)
end)

-- ============================================================================
-- Custom Detection Trigger (for external scripts)
-- ============================================================================

-- Allow other resources to trigger custom detections
-- Usage: exports['anticheat-resource']:triggerCustomDetection(playerId, 'detection_type_name', { custom_data })
exports("triggerCustomDetection", function(playerId, detectionType, data)
    if not playerId or not detectionType then return false end
    
    -- Check if detection type is enabled
    if not API.isDetectionEnabled(detectionType) then
        Logger.debug("custom", "detection_disabled", {
            type = detectionType,
            player = GetPlayerName(playerId)
        })
        return false
    end
    
    local severity = API.getDetectionSeverity(detectionType)
    local action = API.getDetectionAction(detectionType)
    local identifiers = GetPlayerIdentifiers(playerId)
    
    -- Log detection
    Logger.detection(detectionType, severity, 
        API.getPrimaryIdentifier(identifiers), 
        GetPlayerName(playerId), 
        data or {})
    
    -- Report to API
    API.reportDetection(playerId, identifiers, detectionType, severity, data or {})
    
    -- Trigger sanction based on configured action
    TriggerEvent("anticheat:sanction", playerId, action, detectionType, data)
    
    return true
end)

-- ============================================================================
-- Sanction System
-- ============================================================================

AddEventHandler("anticheat:sanction", function(playerId, sanctionType, reason, details)
    if not Config.Sanctions.AutoSanctionEnabled then
        Logger.info("sanctions", "auto_sanction_disabled", {
            player = GetPlayerName(playerId),
            would_be = sanctionType,
            reason = reason
        })
        return
    end
    
    local playerName = GetPlayerName(playerId)
    local identifiers = GetPlayerIdentifiers(playerId)
    local license = API.getPrimaryIdentifier(identifiers)
    
    -- Try to get action from detection type config if reason matches a detection type
    local detectionTypeConfig = API.getDetectionType(reason)
    if detectionTypeConfig and detectionTypeConfig.action then
        sanctionType = detectionTypeConfig.action
    end
    
    -- Track total detections
    if playerStates[playerId] then
        playerStates[playerId].totalDetections = (playerStates[playerId].totalDetections or 0) + 1
        
        -- Auto-escalate to ban if threshold reached
        if playerStates[playerId].totalDetections >= Config.Sanctions.AutoBanThreshold then
            sanctionType = "ban"
            reason = reason .. " (Auto-escalated due to repeated violations)"
        end
    end
    
    -- Execute sanction
    if sanctionType == "warn" then
        warnPlayer(playerId, reason)
    elseif sanctionType == "kick" then
        kickPlayer(playerId, reason)
    elseif sanctionType == "ban" then
        banPlayer(playerId, reason, Config.Sanctions.BanDurations.standard)
    end
    
    -- Log sanction
    Logger.sanction(sanctionType, license, playerName, reason)
    
    -- Report to API
    API.reportSanction(playerId, identifiers, sanctionType, reason, 
        sanctionType == "ban" and Config.Sanctions.BanDurations.standard or 0)
end)

function warnPlayer(playerId, reason)
    TriggerClientEvent("anticheat:warn", playerId, reason)
    Logger.info("sanctions", "warned", {reason = reason}, 
        API.getPrimaryIdentifier(GetPlayerIdentifiers(playerId)), 
        GetPlayerName(playerId))
end

function kickPlayer(playerId, reason)
    local message = string.format(Config.Sanctions.KickMessage, reason)
    DropPlayer(playerId, message)
end

function banPlayer(playerId, reason, duration)
    local identifiers = GetPlayerIdentifiers(playerId)
    local license = API.getPrimaryIdentifier(identifiers)
    local expiresAt = duration > 0 and (os.time() + duration) or 0
    
    -- Add to local cache
    bannedPlayers[license] = {
        reason = reason,
        expires_at = expiresAt,
        banned_at = os.time()
    }
    
    -- Format duration for message
    local durationStr = "Permanent"
    if duration > 0 then
        local days = math.floor(duration / 86400)
        durationStr = days .. " day(s)"
    end
    
    local message = string.format(Config.Sanctions.BanMessage, reason, durationStr)
    DropPlayer(playerId, message)
end

-- ============================================================================
-- Whitelist Check
-- ============================================================================

function isWhitelisted(playerId, identifiers)
    -- Check license whitelist
    for _, id in pairs(identifiers) do
        for _, exempt in ipairs(Config.Whitelist.ExemptLicenses) do
            if id == exempt then return true end
        end
        for _, exempt in ipairs(Config.Whitelist.ExemptSteamIds) do
            if id == exempt then return true end
        end
    end
    
    -- Check ACE groups
    for _, group in ipairs(Config.Whitelist.ExemptAceGroups) do
        if IsPlayerAceAllowed(playerId, group) then
            return true
        end
    end
    
    return false
end

-- ============================================================================
-- Admin Commands (if ACE allowed)
-- ============================================================================

RegisterCommand("ac_status", function(source, args)
    if source > 0 and not IsPlayerAceAllowed(source, "anticheat.admin") then
        return
    end
    
    local status = {
        api_connected = API.isConnected(),
        players_tracked = 0,
        detections_modules = {}
    }
    
    for _ in pairs(playerStates) do
        status.players_tracked = status.players_tracked + 1
    end
    
    for name, config in pairs(Config.Detections) do
        status.detections_modules[name] = config.Enabled
    end
    
    print("^2[Anticheat Status]^7")
    print("API Connected: " .. tostring(status.api_connected))
    print("Players Tracked: " .. status.players_tracked)
    print("Detection Modules:")
    for name, enabled in pairs(status.detections_modules) do
        print("  " .. name .. ": " .. (enabled and "^2Enabled^7" or "^1Disabled^7"))
    end
end, false)

RegisterCommand("ac_kick", function(source, args)
    if source > 0 and not IsPlayerAceAllowed(source, "anticheat.admin") then
        return
    end
    
    local targetId = tonumber(args[1])
    local reason = table.concat(args, " ", 2) or "Admin kick"
    
    if targetId and GetPlayerName(targetId) then
        kickPlayer(targetId, reason)
        Logger.info("admin", "manual_kick", {
            target = GetPlayerName(targetId),
            admin = source > 0 and GetPlayerName(source) or "Console",
            reason = reason
        })
    else
        print("Usage: ac_kick [player_id] [reason]")
    end
end, false)

RegisterCommand("ac_ban", function(source, args)
    if source > 0 and not IsPlayerAceAllowed(source, "anticheat.admin") then
        return
    end
    
    local targetId = tonumber(args[1])
    local duration = tonumber(args[2]) or 86400 -- Default 1 day
    local reason = table.concat(args, " ", 3) or "Admin ban"
    
    if targetId and GetPlayerName(targetId) then
        banPlayer(targetId, reason, duration)
        Logger.info("admin", "manual_ban", {
            target = GetPlayerName(targetId),
            admin = source > 0 and GetPlayerName(source) or "Console",
            reason = reason,
            duration = duration
        })
    else
        print("Usage: ac_ban [player_id] [duration_seconds] [reason]")
    end
end, false)

Logger.info("main", "server_script_loaded", {})
