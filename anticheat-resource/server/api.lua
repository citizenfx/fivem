--[[
    Anticheat System - Server API Client
    HTTP communication with web admin backend
]]

API = {}

local apiKey = nil
local currentRuleset = nil
local isConnected = false

-- Initialize API connection
function API.init()
    if not Config.API.Enabled then
        Logger.info("api", "disabled", {})
        return
    end
    
    -- Get API key from convar
    apiKey = GetConvar(Config.API.ApiKeyConvar, "")
    if apiKey == "" then
        Logger.warn("api", "no_api_key", {
            message = "API key not set. Set '" .. Config.API.ApiKeyConvar .. "' in server.cfg"
        })
        return
    end
    
    -- Test connection and fetch initial ruleset
    API.healthCheck(function(success)
        if success then
            isConnected = true
            Logger.info("api", "connected", {url = Config.API.BaseURL})
            API.fetchRuleset()
        else
            Logger.error("api", "connection_failed", {url = Config.API.BaseURL})
        end
    end)
    
    -- Set up periodic ruleset sync
    CreateThread(function()
        while true do
            Wait(Config.API.RulesetSyncInterval * 1000)
            if isConnected then
                API.fetchRuleset()
            end
        end
    end)
end

-- Make HTTP request to API
function API.request(endpoint, method, data, callback)
    if not Config.API.Enabled then
        if callback then callback(false, nil) end
        return
    end
    
    local url = Config.API.BaseURL .. endpoint
    local headers = {
        ["Content-Type"] = "application/json",
        ["X-API-Key"] = apiKey or ""
    }
    
    local body = data and json.encode(data) or ""
    
    PerformHttpRequest(url, function(statusCode, responseText, responseHeaders)
        local success = statusCode >= 200 and statusCode < 300
        local responseData = nil
        
        if responseText and responseText ~= "" then
            local ok, decoded = pcall(json.decode, responseText)
            if ok then
                responseData = decoded
            end
        end
        
        if Config.Debug then
            Logger.debug("api", "response", {
                endpoint = endpoint,
                status = statusCode,
                success = success
            })
        end
        
        if callback then
            callback(success, responseData, statusCode)
        end
    end, method, body, headers)
end

-- Health check
function API.healthCheck(callback)
    API.request("/health", "GET", nil, function(success, data)
        callback(success and data and data.status == "ok")
    end)
end

-- Fetch active ruleset
function API.fetchRuleset(callback)
    API.request("/rulesets/active", "GET", nil, function(success, data)
        if success and data then
            currentRuleset = data
            Logger.info("api", "ruleset_updated", {
                name = data.name,
                version = data.version or "unknown"
            })
            -- Apply ruleset to config
            API.applyRuleset(data)
        end
        if callback then callback(success, data) end
    end)
end

-- Apply fetched ruleset to runtime config
function API.applyRuleset(ruleset)
    if not ruleset then return end
    
    local configChanged = false
    
    -- Handle new config structure: { detections: { speed: {...}, health: {...} }, sanctions: {...} }
    if ruleset.detections then
        for moduleName, moduleConfig in pairs(ruleset.detections) do
            -- Capitalize first letter to match Config.Detections keys
            local configKey = moduleName:gsub("^%l", string.upper)
            
            if Config.Detections[configKey] then
                for key, value in pairs(moduleConfig) do
                    -- Convert camelCase/snake_case to PascalCase for Lua config
                    local luaKey = key:gsub("_(%l)", function(c) return c:upper() end)
                    luaKey = luaKey:gsub("^%l", string.upper)
                    
                    if Config.Detections[configKey][luaKey] ~= nil then
                        Config.Detections[configKey][luaKey] = value
                        configChanged = true
                    end
                end
            end
        end
    end
    
    -- Legacy support: handle 'modules' structure
    if ruleset.modules then
        for moduleName, moduleConfig in pairs(ruleset.modules) do
            local configKey = moduleName:gsub("_detection", ""):gsub("^%l", string.upper)
            
            if Config.Detections[configKey] then
                for key, value in pairs(moduleConfig) do
                    local luaKey = key:gsub("_(%l)", function(c) return c:upper() end)
                    luaKey = luaKey:gsub("^%l", string.upper)
                    
                    if Config.Detections[configKey][luaKey] ~= nil then
                        Config.Detections[configKey][luaKey] = value
                        configChanged = true
                    end
                end
            end
        end
    end
    
    -- Apply sanctions settings
    if ruleset.sanctions then
        if ruleset.sanctions.autoKickThreshold then
            Config.Sanctions.AutoKickThreshold = ruleset.sanctions.autoKickThreshold
        end
        if ruleset.sanctions.autoBanThreshold then
            Config.Sanctions.AutoBanThreshold = ruleset.sanctions.autoBanThreshold
        end
        if ruleset.sanctions.defaultBanDuration then
            Config.Sanctions.DefaultBanDuration = ruleset.sanctions.defaultBanDuration
        end
    end
    
    -- Apply global settings
    if ruleset.global_settings then
        if ruleset.global_settings.log_level then
            Config.Logging.Level = ruleset.global_settings.log_level
        end
        if ruleset.global_settings.notify_discord_webhook then
            Config.Logging.DiscordWebhook = ruleset.global_settings.notify_discord_webhook
        end
        if ruleset.global_settings.notify_discord ~= nil then
            Config.Logging.DiscordEnabled = ruleset.global_settings.notify_discord
        end
    end
    
    -- Apply detection types and severities
    if ruleset.detection_types then
        Config.DetectionTypes = ruleset.detection_types
        configChanged = true
        Logger.debug("api", "detection_types_loaded", {
            count = API.tableLength(ruleset.detection_types)
        })
    end
    
    if configChanged then
        Logger.info("api", "config_applied", {
            ruleset_name = ruleset.name,
            ruleset_version = ruleset.version
        })
        -- Trigger config reload event for detection modules
        TriggerEvent("anticheat:configReloaded", ruleset)
    end
end

-- Get detection type config
function API.getDetectionType(typeName)
    if Config.DetectionTypes and Config.DetectionTypes[typeName] then
        return Config.DetectionTypes[typeName]
    end
    -- Default fallback
    return { severity = "medium", action = "warn", enabled = true }
end

-- Check if detection type is enabled
function API.isDetectionEnabled(typeName)
    local typeConfig = API.getDetectionType(typeName)
    return typeConfig.enabled ~= false
end

-- Get severity for detection type
function API.getDetectionSeverity(typeName)
    local typeConfig = API.getDetectionType(typeName)
    return typeConfig.severity or "medium"
end

-- Get action for detection type
function API.getDetectionAction(typeName)
    local typeConfig = API.getDetectionType(typeName)
    return typeConfig.action or "warn"
end

-- Helper: Count table entries
function API.tableLength(t)
    local count = 0
    for _ in pairs(t or {}) do count = count + 1 end
    return count
end

-- Report detection to API
function API.reportDetection(playerId, playerIdentifiers, detectionType, severity, data)
    local payload = {
        player_license = API.getPrimaryIdentifier(playerIdentifiers),
        player_name = GetPlayerName(playerId),
        detection_type = detectionType,
        severity = severity,
        data = data,
        timestamp = os.date("!%Y-%m-%dT%H:%M:%SZ")
    }
    
    API.request("/detections", "POST", payload, function(success)
        if not success then
            Logger.warn("api", "detection_report_failed", payload)
        end
    end)
end

-- Report sanction to API
function API.reportSanction(playerId, playerIdentifiers, sanctionType, reason, duration)
    local payload = {
        player_license = API.getPrimaryIdentifier(playerIdentifiers),
        player_name = GetPlayerName(playerId),
        sanction_type = sanctionType,
        reason = reason,
        duration = duration,
        timestamp = os.date("!%Y-%m-%dT%H:%M:%SZ")
    }
    
    API.request("/sanctions", "POST", payload, function(success)
        if not success then
            Logger.warn("api", "sanction_report_failed", payload)
        end
    end)
end

-- Get player info from API
function API.getPlayerInfo(license, callback)
    API.request("/players/" .. license, "GET", nil, callback)
end

-- Sync player connection
function API.playerConnected(playerId, playerIdentifiers)
    local payload = {
        license = API.getPrimaryIdentifier(playerIdentifiers),
        name = GetPlayerName(playerId),
        identifiers = playerIdentifiers
    }
    
    API.request("/players/connect", "POST", payload)
end

-- Sync player disconnection
function API.playerDisconnected(playerId, playerIdentifiers, reason)
    local payload = {
        license = API.getPrimaryIdentifier(playerIdentifiers),
        reason = reason
    }
    
    API.request("/players/disconnect", "POST", payload)
end

-- Get primary identifier (license preferred)
function API.getPrimaryIdentifier(identifiers)
    if type(identifiers) == "string" then return identifiers end
    
    for _, id in pairs(identifiers or {}) do
        if string.find(id, "license:") then
            return id
        end
    end
    for _, id in pairs(identifiers or {}) do
        if string.find(id, "steam:") then
            return id
        end
    end
    return identifiers[1] or "unknown"
end

-- Get current ruleset
function API.getCurrentRuleset()
    return currentRuleset
end

-- Check if connected
function API.isConnected()
    return isConnected
end
