--[[
    Anticheat System - Weapon Detection
    Detects blacklisted weapons, invalid ammo, and rapid fire
]]

WeaponDetection = {}

local playerData = {}

-- Initialize player tracking
function WeaponDetection.initPlayer(playerId)
    playerData[playerId] = {
        lastWeapon = nil,
        lastAmmo = 0,
        shotTimestamps = {},
        violations = 0
    }
end

-- Clean up player data
function WeaponDetection.cleanupPlayer(playerId)
    playerData[playerId] = nil
end

-- Check player weapons
function WeaponDetection.check(playerId, weaponHash, ammo)
    if not Config.Detections.Weapons.Enabled then return end
    
    local data = playerData[playerId]
    if not data then
        WeaponDetection.initPlayer(playerId)
        data = playerData[playerId]
    end
    
    local playerName = GetPlayerName(playerId)
    local identifiers = GetPlayerIdentifiers(playerId)
    
    -- Check for blacklisted weapons
    for _, blacklistedHash in ipairs(Config.Detections.Weapons.BlacklistedWeapons) do
        if weaponHash == blacklistedHash then
            WeaponDetection.onViolation(playerId, "blacklisted_weapon", {
                weapon_hash = weaponHash,
                weapon_name = "Unknown" -- Would need weapon name lookup
            })
            return
        end
    end
    
    -- Check for invalid ammo (if max is set)
    if Config.Detections.Weapons.MaxAmmo > 0 and ammo > Config.Detections.Weapons.MaxAmmo then
        WeaponDetection.onViolation(playerId, "invalid_ammo", {
            current_ammo = ammo,
            max_allowed = Config.Detections.Weapons.MaxAmmo,
            weapon_hash = weaponHash
        })
    end
    
    data.lastWeapon = weaponHash
    data.lastAmmo = ammo
end

-- Track weapon fire for rapid fire detection
function WeaponDetection.onWeaponFire(playerId, weaponHash)
    if not Config.Detections.Weapons.Enabled then return end
    
    local data = playerData[playerId]
    if not data then return end
    
    local currentTime = GetGameTimer()
    
    -- Add current shot timestamp
    table.insert(data.shotTimestamps, currentTime)
    
    -- Remove shots older than 1 second
    local cutoff = currentTime - 1000
    local newTimestamps = {}
    for _, timestamp in ipairs(data.shotTimestamps) do
        if timestamp > cutoff then
            table.insert(newTimestamps, timestamp)
        end
    end
    data.shotTimestamps = newTimestamps
    
    -- Check for rapid fire
    if #data.shotTimestamps > Config.Detections.Weapons.RapidFireThreshold then
        WeaponDetection.onViolation(playerId, "rapid_fire", {
            shots_per_second = #data.shotTimestamps,
            threshold = Config.Detections.Weapons.RapidFireThreshold,
            weapon_hash = weaponHash
        })
        data.shotTimestamps = {} -- Reset after violation
    end
end

-- Handle violation
function WeaponDetection.onViolation(playerId, violationType, details)
    local data = playerData[playerId]
    if not data then return end
    
    data.violations = data.violations + 1
    local playerName = GetPlayerName(playerId)
    local identifiers = GetPlayerIdentifiers(playerId)
    
    Logger.detection("weapon_" .. violationType, 
        API.getPrimaryIdentifier(identifiers), 
        playerName, 
        details
    )
    
    -- Check if threshold reached
    if data.violations >= Config.Detections.Weapons.GraceViolations then
        TriggerEvent("anticheat:sanction", playerId, Config.Detections.Weapons.Action, 
            "Weapon violation: " .. violationType, details)
        data.violations = 0
    end
    
    -- Report to API
    API.reportDetection(playerId, identifiers, "weapon_" .. violationType, "medium", details)
end
