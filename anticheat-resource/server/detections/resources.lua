--[[
    Anticheat System - Resource Detection
    Detects unauthorized resource injection
]]

ResourceDetection = {}

local expectedResources = {}
local initialResourceList = {}
local initialized = false

-- Initialize resource tracking
function ResourceDetection.init()
    if not Config.Detections.Resources.Enabled then return end
    
    -- Store initial resource list
    local numResources = GetNumResources()
    for i = 0, numResources - 1 do
        local resourceName = GetResourceByFindIndex(i)
        if resourceName then
            initialResourceList[resourceName] = true
        end
    end
    
    Logger.info("resources", "initialized", {
        resource_count = numResources
    })
    
    initialized = true
end

-- Check for resource anomalies
function ResourceDetection.check()
    if not Config.Detections.Resources.Enabled or not initialized then return end
    
    local currentResources = {}
    local numResources = GetNumResources()
    
    for i = 0, numResources - 1 do
        local resourceName = GetResourceByFindIndex(i)
        if resourceName then
            currentResources[resourceName] = true
            
            -- Check for blacklisted patterns
            for _, pattern in ipairs(Config.Detections.Resources.BlacklistedPatterns) do
                if string.match(resourceName, pattern) then
                    ResourceDetection.onViolation("blacklisted_resource", {
                        resource_name = resourceName,
                        pattern = pattern
                    })
                end
            end
            
            -- Check for new resources
            if not initialResourceList[resourceName] then
                Logger.warn("resources", "new_resource_detected", {
                    resource_name = resourceName
                })
            end
        end
    end
    
    -- Check resource count (if expected count is set)
    if Config.Detections.Resources.ExpectedResourceCount > 0 then
        if numResources > Config.Detections.Resources.ExpectedResourceCount then
            ResourceDetection.onViolation("unexpected_resource_count", {
                current_count = numResources,
                expected_count = Config.Detections.Resources.ExpectedResourceCount
            })
        end
    end
end

-- Handle violation
function ResourceDetection.onViolation(violationType, details)
    Logger.detection("resource_" .. violationType, 
        nil, 
        "SERVER", 
        details,
        "error"
    )
    
    -- Resource violations are server-wide, not player-specific
    -- This is more of an alert than a sanction
    
    -- Report to API
    if API.isConnected() then
        API.request("/detections", "POST", {
            detection_type = "resource_" .. violationType,
            severity = "critical",
            data = details,
            timestamp = os.date("!%Y-%m-%dT%H:%M:%SZ")
        })
    end
end

-- Check client resources (called from client)
RegisterNetEvent("anticheat:reportClientResources")
AddEventHandler("anticheat:reportClientResources", function(clientResources)
    if not Config.Detections.Resources.Enabled then return end
    
    local playerId = source
    local playerName = GetPlayerName(playerId)
    local identifiers = GetPlayerIdentifiers(playerId)
    
    -- Check for blacklisted client-side resources
    for _, resourceName in ipairs(clientResources or {}) do
        for _, pattern in ipairs(Config.Detections.Resources.BlacklistedPatterns) do
            if string.match(resourceName, pattern) then
                Logger.detection("resource_client_blacklisted", 
                    API.getPrimaryIdentifier(identifiers), 
                    playerName, 
                    {
                        resource_name = resourceName,
                        pattern = pattern
                    }
                )
                
                TriggerEvent("anticheat:sanction", playerId, Config.Detections.Resources.Action, 
                    "Blacklisted client resource: " .. resourceName, {
                        resource = resourceName
                    })
                
                API.reportDetection(playerId, identifiers, "resource_client_blacklisted", "critical", {
                    resource_name = resourceName
                })
                return
            end
        end
    end
end)

-- Periodic resource check
CreateThread(function()
    Wait(5000) -- Wait for server to fully start
    ResourceDetection.init()
    
    while true do
        Wait(30000) -- Check every 30 seconds
        ResourceDetection.check()
    end
end)
