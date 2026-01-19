--[[
    Anticheat System - Client Script
    Collects and reports player state to server
]]

local isInitialized = false
local lastReportTime = 0

-- ============================================================================
-- Initialization
-- ============================================================================

CreateThread(function()
    -- Wait for game to fully load
    while not NetworkIsSessionStarted() do
        Wait(100)
    end
    
    Wait(2000) -- Additional delay for stability
    isInitialized = true
    
    -- Start state reporting loop
    startReportingLoop()
end)

-- ============================================================================
-- State Reporting
-- ============================================================================

function startReportingLoop()
    CreateThread(function()
        while true do
            Wait(Config.ClientReporting.ReportInterval)
            
            if isInitialized then
                reportState()
            end
        end
    end)
end

function reportState()
    local playerPed = PlayerPedId()
    if not DoesEntityExist(playerPed) then return end
    
    local state = {}
    
    -- Position data
    if Config.ClientReporting.ReportPosition then
        local coords = GetEntityCoords(playerPed)
        state.position = {
            x = coords.x,
            y = coords.y,
            z = coords.z
        }
    end
    
    -- Vehicle data
    if Config.ClientReporting.ReportVehicle then
        local vehicle = GetVehiclePedIsIn(playerPed, false)
        state.inVehicle = vehicle ~= 0
        if state.inVehicle then
            state.vehicleSpeed = GetEntitySpeed(vehicle)
            state.vehicleModel = GetEntityModel(vehicle)
        end
    end
    
    -- Health data
    if Config.ClientReporting.ReportHealth then
        state.health = GetEntityHealth(playerPed)
        state.armor = GetPedArmour(playerPed)
        state.maxHealth = GetEntityMaxHealth(playerPed)
    end
    
    -- Weapon data
    if Config.ClientReporting.ReportWeapons then
        local _, weaponHash = GetCurrentPedWeapon(playerPed, true)
        state.weapon = weaponHash
        state.ammo = GetAmmoInPedWeapon(playerPed, weaponHash)
    end
    
    -- Send to server
    TriggerServerEvent("anticheat:reportState", state)
end

-- ============================================================================
-- Weapon Fire Detection
-- ============================================================================

CreateThread(function()
    while true do
        Wait(0)
        
        if isInitialized then
            local playerPed = PlayerPedId()
            
            if IsPedShooting(playerPed) then
                local _, weaponHash = GetCurrentPedWeapon(playerPed, true)
                TriggerServerEvent("anticheat:weaponFired", weaponHash)
            end
        end
    end
end)

-- ============================================================================
-- Resource List Reporting
-- ============================================================================

CreateThread(function()
    Wait(5000) -- Wait for resources to load
    
    local clientResources = {}
    local numResources = GetNumResources()
    
    for i = 0, numResources - 1 do
        local resourceName = GetResourceByFindIndex(i)
        if resourceName then
            table.insert(clientResources, resourceName)
        end
    end
    
    TriggerServerEvent("anticheat:reportClientResources", clientResources)
end)

-- ============================================================================
-- Warning Display Handler
-- ============================================================================

RegisterNetEvent("anticheat:warn")
AddEventHandler("anticheat:warn", function(reason)
    -- Display warning notification
    BeginTextCommandThefeedPost("STRING")
    AddTextComponentSubstringPlayerName("~r~ANTICHEAT WARNING~s~\n" .. reason)
    EndTextCommandThefeedPostTicker(true, true)
    
    -- Also show in center of screen
    SetTextFont(4)
    SetTextScale(0.5, 0.5)
    SetTextColour(255, 100, 100, 255)
    SetTextCentre(true)
    SetTextEntry("STRING")
    AddTextComponentString("[ANTICHEAT] Warning: " .. reason)
    DrawText(0.5, 0.3)
end)

-- ============================================================================
-- Anti-Tamper (Basic)
-- ============================================================================

-- Detect if this script is being modified
local scriptHash = nil

CreateThread(function()
    Wait(1000)
    -- Store initial script state
    scriptHash = GetHashKey(GetCurrentResourceName())
end)

-- Periodic integrity check
CreateThread(function()
    while true do
        Wait(10000)
        
        -- Basic check - ensure the resource is still running correctly
        if GetCurrentResourceName() ~= "anticheat-resource" then
            -- Resource name mismatch - possible tampering
            TriggerServerEvent("anticheat:reportClientResources", {"TAMPER_DETECTED"})
        end
    end
end)

-- ============================================================================
-- Debug Mode
-- ============================================================================

if Config.Debug then
    RegisterCommand("ac_debug", function()
        local ped = PlayerPedId()
        local coords = GetEntityCoords(ped)
        local health = GetEntityHealth(ped)
        local armor = GetPedArmour(ped)
        local _, weapon = GetCurrentPedWeapon(ped, true)
        
        print("=== Anticheat Debug ===")
        print("Position: " .. coords.x .. ", " .. coords.y .. ", " .. coords.z)
        print("Health: " .. health)
        print("Armor: " .. armor)
        print("Weapon Hash: " .. weapon)
        print("In Vehicle: " .. tostring(GetVehiclePedIsIn(ped, false) ~= 0))
    end, false)
end
