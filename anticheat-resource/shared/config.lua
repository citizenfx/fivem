--[[
    Anticheat System - Shared Configuration
    This file is loaded on both client and server
]]

Config = {}

-- ============================================================================
-- API Configuration
-- ============================================================================
Config.API = {
    -- Web admin backend URL
    BaseURL = "http://localhost:3000/api/fivem",
    -- API key for authentication (set this in your server.cfg as a convar)
    -- set anticheat_api_key "your-secret-key-here"
    ApiKeyConvar = "anticheat_api_key",
    -- Sync interval for fetching updated rulesets (in seconds)
    RulesetSyncInterval = 300,
    -- Enable/disable API connectivity
    Enabled = true
}

-- ============================================================================
-- Logging Configuration
-- ============================================================================
Config.Logging = {
    -- Log levels: debug, info, warn, error
    Level = "info",
    -- File path for JSON logs (relative to server data folder)
    FilePath = "anticheat.log",
    -- Include timestamps in console output
    ConsoleTimestamps = true,
    -- Discord webhook for critical alerts (optional)
    DiscordWebhook = nil
}

-- ============================================================================
-- Detection Modules Configuration
-- ============================================================================
Config.Detections = {
    -- Speed/Teleportation Detection
    Speed = {
        Enabled = true,
        -- Maximum foot speed (m/s) - normal sprint is ~7-8 m/s
        MaxFootSpeed = 15.0,
        -- Maximum vehicle speed (m/s) - ~300 km/h = 83 m/s, with buffer
        MaxVehicleSpeed = 100.0,
        -- Distance threshold for teleport detection (meters)
        TeleportThreshold = 100.0,
        -- Check interval (ms)
        CheckInterval = 1000,
        -- Number of violations before action
        GraceViolations = 3,
        -- Action: "warn", "kick", "ban"
        Action = "kick"
    },

    -- Health/Godmode Detection
    Health = {
        Enabled = true,
        -- Maximum allowed health
        MaxHealth = 200,
        -- Maximum allowed armor
        MaxArmor = 100,
        -- Minimum health (to detect negative health exploits)
        MinHealth = 0,
        -- Godmode detection - interval to check if player takes damage
        GodmodeCheckInterval = 5000,
        -- Check interval (ms)
        CheckInterval = 2000,
        GraceViolations = 2,
        Action = "kick"
    },

    -- Weapon Detection
    Weapons = {
        Enabled = true,
        -- Blacklisted weapon hashes (add weapon hash numbers here)
        BlacklistedWeapons = {
            -- Example: GetHashKey("WEAPON_RAILGUN")
        },
        -- Maximum ammo per weapon type (0 = unlimited allowed)
        MaxAmmo = 0,
        -- Detect rapid fire (shots per second threshold)
        RapidFireThreshold = 20,
        -- Check interval (ms)
        CheckInterval = 500,
        GraceViolations = 2,
        Action = "kick"
    },

    -- Entity Spawning Detection
    Entities = {
        Enabled = true,
        -- Use server-side entity lockdown
        EntityLockdown = true,
        -- Allowed entity types for clients (empty = none allowed)
        AllowedClientSpawns = {},
        -- Blacklisted model hashes
        BlacklistedModels = {},
        -- Maximum entities per player
        MaxEntitiesPerPlayer = 10,
        Action = "kick"
    },

    -- Event Validation
    Events = {
        Enabled = true,
        -- Rate limit: max events per second per player
        RateLimitPerSecond = 20,
        -- Blocked event names (common exploitation targets)
        BlockedEvents = {
            "esx:getSharedObject",
            "esx:setJob",
            "esx:setAccountMoney"
        },
        -- Events that require validation callbacks
        ValidatedEvents = {},
        GraceViolations = 3,
        Action = "kick"
    },

    -- Resource Injection Detection
    Resources = {
        Enabled = true,
        -- Expected resource count (set to 0 to disable)
        ExpectedResourceCount = 0,
        -- Blacklisted resource name patterns
        BlacklistedPatterns = {
            "^m_",      -- Common menu patterns
            "^admin_",
            "^cheat_",
            "^hack_"
        },
        Action = "ban"
    }
}

-- ============================================================================
-- Sanction Configuration
-- ============================================================================
Config.Sanctions = {
    -- Enable automatic sanctions
    AutoSanctionEnabled = true,
    -- Ban durations (in seconds, 0 = permanent)
    BanDurations = {
        ["temporary"] = 86400,      -- 24 hours
        ["standard"] = 604800,      -- 7 days
        ["severe"] = 2592000,       -- 30 days
        ["permanent"] = 0           -- Permanent
    },
    -- Auto-ban after this many total detections
    AutoBanThreshold = 10,
    -- Kick message template
    KickMessage = "[Anticheat] You have been kicked for: %s",
    -- Ban message template
    BanMessage = "[Anticheat] You have been banned. Reason: %s | Duration: %s"
}

-- ============================================================================
-- Whitelist Configuration
-- ============================================================================
Config.Whitelist = {
    -- Exempt players by license identifier
    ExemptLicenses = {},
    -- Exempt players by Steam hex
    ExemptSteamIds = {},
    -- Exempt admin groups (if using ACE permissions)
    ExemptAceGroups = {
        "group.admin",
        "group.moderator"
    }
}

-- ============================================================================
-- Client Reporting Configuration
-- ============================================================================
Config.ClientReporting = {
    -- How often client sends state to server (ms)
    ReportInterval = 2000,
    -- Include position data
    ReportPosition = true,
    -- Include vehicle data
    ReportVehicle = true,
    -- Include weapon data
    ReportWeapons = true,
    -- Include health data
    ReportHealth = true
}

-- ============================================================================
-- Debug Mode
-- ============================================================================
Config.Debug = false
