--[[
    Anticheat System - Server Logger
    Structured JSON logging for ELK Stack integration
]]

Logger = {}

local logLevels = {
    debug = 1,
    info = 2,
    warn = 3,
    error = 4
}

local currentLogLevel = logLevels[Config.Logging.Level] or logLevels.info
local logFile = nil

-- Initialize log file
local function initLogFile()
    if logFile then return end
    local filePath = Config.Logging.FilePath
    logFile = io.open(filePath, "a")
    if not logFile then
        print("[Anticheat] Warning: Could not open log file: " .. filePath)
    end
end

-- Format timestamp for logging
local function getTimestamp()
    return os.date("!%Y-%m-%dT%H:%M:%S") .. ".000Z"
end

-- Escape string for JSON
local function jsonEscape(str)
    if type(str) ~= "string" then return tostring(str) end
    return str:gsub('\\', '\\\\')
              :gsub('"', '\\"')
              :gsub('\n', '\\n')
              :gsub('\r', '\\r')
              :gsub('\t', '\\t')
end

-- Simple JSON encoder for logging
local function toJson(tbl)
    if type(tbl) ~= "table" then
        if type(tbl) == "string" then
            return '"' .. jsonEscape(tbl) .. '"'
        elseif type(tbl) == "boolean" then
            return tbl and "true" or "false"
        elseif tbl == nil then
            return "null"
        else
            return tostring(tbl)
        end
    end

    local isArray = #tbl > 0
    local parts = {}
    
    if isArray then
        for _, v in ipairs(tbl) do
            table.insert(parts, toJson(v))
        end
        return "[" .. table.concat(parts, ",") .. "]"
    else
        for k, v in pairs(tbl) do
            table.insert(parts, '"' .. jsonEscape(k) .. '":' .. toJson(v))
        end
        return "{" .. table.concat(parts, ",") .. "}"
    end
end

-- Core logging function
function Logger.log(level, module, event, data, playerId, playerName)
    -- Check log level
    if logLevels[level] < currentLogLevel then return end
    
    -- Build log entry
    local entry = {
        timestamp = getTimestamp(),
        level = level,
        module = module,
        event = event,
        player_id = playerId or nil,
        player_name = playerName or nil,
        data = data or {}
    }
    
    local jsonLine = toJson(entry)
    
    -- Write to file
    initLogFile()
    if logFile then
        logFile:write(jsonLine .. "\n")
        logFile:flush()
    end
    
    -- Write to console
    local consolePrefix = Config.Logging.ConsoleTimestamps 
        and ("[" .. getTimestamp() .. "] ") 
        or ""
    
    local levelColors = {
        debug = "^5",  -- Purple
        info = "^2",   -- Green
        warn = "^3",   -- Yellow
        error = "^1"   -- Red
    }
    
    local color = levelColors[level] or "^7"
    print(consolePrefix .. color .. "[Anticheat:" .. level:upper() .. "] ^7" .. module .. " - " .. event)
    
    -- Discord webhook for critical events
    if level == "error" or level == "warn" then
        Logger.sendDiscordAlert(level, module, event, data, playerName)
    end
end

-- Convenience methods
function Logger.debug(module, event, data, playerId, playerName)
    Logger.log("debug", module, event, data, playerId, playerName)
end

function Logger.info(module, event, data, playerId, playerName)
    Logger.log("info", module, event, data, playerId, playerName)
end

function Logger.warn(module, event, data, playerId, playerName)
    Logger.log("warn", module, event, data, playerId, playerName)
end

function Logger.error(module, event, data, playerId, playerName)
    Logger.log("error", module, event, data, playerId, playerName)
end

-- Detection-specific logging
function Logger.detection(detectionType, playerId, playerName, details, severity)
    Logger.log(
        severity or "warn",
        "detection",
        detectionType,
        details,
        playerId,
        playerName
    )
end

-- Sanction logging
function Logger.sanction(sanctionType, playerId, playerName, reason, duration)
    Logger.log(
        "info",
        "sanction",
        sanctionType,
        {
            reason = reason,
            duration = duration
        },
        playerId,
        playerName
    )
end

-- Discord webhook alert
function Logger.sendDiscordAlert(level, module, event, data, playerName)
    local webhook = Config.Logging.DiscordWebhook
    if not webhook then return end
    
    local color = level == "error" and 15158332 or 15105570 -- Red or Orange
    
    local embed = {
        title = "[Anticheat] " .. event,
        description = "Module: " .. module,
        color = color,
        fields = {
            {name = "Player", value = playerName or "Unknown", inline = true},
            {name = "Level", value = level:upper(), inline = true}
        },
        timestamp = getTimestamp()
    }
    
    if data then
        table.insert(embed.fields, {
            name = "Details",
            value = "```json\n" .. toJson(data):sub(1, 500) .. "\n```",
            inline = false
        })
    end
    
    PerformHttpRequest(webhook, function() end, "POST", json.encode({embeds = {embed}}), {
        ["Content-Type"] = "application/json"
    })
end

-- Cleanup on resource stop
AddEventHandler("onResourceStop", function(resourceName)
    if GetCurrentResourceName() ~= resourceName then return end
    if logFile then
        logFile:close()
    end
end)

Logger.info("logger", "initialized", {level = Config.Logging.Level})
