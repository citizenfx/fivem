-- in-memory spawnpoint array for this script execution instance
local spawnPoints = {}

-- auto-spawn enabled flag
local autoSpawnEnabled = true
local autoSpawnCallback

-- loads a set of spawn points from a JSON string
function loadSpawns(spawnString)
    -- decode the JSON string
    local data = json.decode(spawnString)

    -- do we have a 'spawns' field?
    if not data.spawns then
        error("no 'spawns' in JSON data")
    end

    -- loop through the spawns
    for i, spawn in ipairs(data.spawns) do
        -- and add it to the list (validating as we go)
        addSpawnPoint(spawn)
    end
end

local spawnNum = 1

function addSpawnPoint(spawn)
    -- validate the spawn (position)
    if not tonumber(spawn.x) or not tonumber(spawn.y) or not tonumber(spawn.z) then
        error("invalid spawn position")
    end

    -- heading
    if not tonumber(spawn.heading) then
        error("invalid spawn heading")
    end

    -- model (try integer first, if not, hash it)
    local model = spawn.model

    if not tonumber(spawn.model) then
        model = GetHashKey(spawn.model)
    end

    -- is the model actually a model?
    if not IsModelInCdimage(model) then
        error("invalid spawn model")
    end

    -- is is even a ped?
    -- not in V?
    --[[if not IsThisModelAPed(model) then
        error("this model ain't a ped!")
    end]]

    -- overwrite the model in case we hashed it
    spawn.model = model

    -- add an index
    spawn.idx = spawnNum
    spawnNum = spawnNum + 1

    -- all OK, add the spawn entry to the list
    table.insert(spawnPoints, spawn)

    return spawn.idx
end

-- removes a spawn point
function removeSpawnPoint(spawn)
    for i = 1, #spawnPoints do
        if spawnPoints[i].idx == spawn then
            table.remove(spawnPoints, i)
            return
        end
    end
end

-- changes the auto-spawn flag
function setAutoSpawn(enabled)
    autoSpawnEnabled = enabled
end

-- sets a callback to execute instead of 'native' spawning when trying to auto-spawn
function setAutoSpawnCallback(cb)
    autoSpawnCallback = cb
    autoSpawnEnabled = true
end

-- function as existing in original R* scripts
local function freezePlayer(id, freeze)
    local player = id
    SetPlayerControl(player, not freeze, false)

    local ped = GetPlayerPed(player)

    if not freeze then
        if not IsEntityVisible(ped) then
            SetEntityVisible(ped, true)
        end

        if not IsPedInAnyVehicle(ped) then
            SetEntityCollision(ped, true)
        end

        FreezeEntityPosition(ped, false)
        --SetCharNeverTargetted(ped, false)
        SetPlayerInvincible(player, false)
    else
        if IsEntityVisible(ped) then
            SetEntityVisible(ped, false)
        end

        SetEntityCollision(ped, false)
        FreezeEntityPosition(ped, true)
        --SetCharNeverTargetted(ped, true)
        SetPlayerInvincible(player, true)
        --RemovePtfxFromPed(ped)

        if not IsPedFatallyInjured(ped) then
            ClearPedTasksImmediately(ped)
        end
    end
end

function loadScene(x, y, z)
	if not NewLoadSceneStart then
		return
	end

    NewLoadSceneStart(x, y, z, 0.0, 0.0, 0.0, 20.0, 0)

    while IsNewLoadSceneActive() do
        networkTimer = GetNetworkTimer()

        NetworkUpdateLoadScene()
    end
end

-- to prevent trying to spawn multiple times
local spawnLock = false

-- spawns the current player at a certain spawn point index (or a random one, for that matter)
function spawnPlayer(spawnIdx, cb)
    if spawnLock then
        return
    end

    spawnLock = true

    Citizen.CreateThread(function()
        -- if the spawn isn't set, select a random one
        if not spawnIdx then
            spawnIdx = GetRandomIntInRange(1, #spawnPoints + 1)
        end

        -- get the spawn from the array
        local spawn

        if type(spawnIdx) == 'table' then
            spawn = spawnIdx
        else
            spawn = spawnPoints[spawnIdx]
        end

        if not spawn.skipFade then
            DoScreenFadeOut(500)

            while not IsScreenFadedOut() do
                Citizen.Wait(0)
            end
        end

        -- validate the index
        if not spawn then
            Citizen.Trace("tried to spawn at an invalid spawn index\n")

            spawnLock = false

            return
        end

        -- freeze the local player
        freezePlayer(PlayerId(), true)

        -- if the spawn has a model set
        if spawn.model then
            RequestModel(spawn.model)

            -- load the model for this spawn
            while not HasModelLoaded(spawn.model) do
                RequestModel(spawn.model)

                Wait(0)
            end

            -- change the player model
            SetPlayerModel(PlayerId(), spawn.model)

            -- release the player model
            SetModelAsNoLongerNeeded(spawn.model)
            
            -- RDR3 player model bits
            if N_0x283978a15512b2fe then
				N_0x283978a15512b2fe(PlayerPedId(), true)
            end
        end

        -- preload collisions for the spawnpoint
        RequestCollisionAtCoord(spawn.x, spawn.y, spawn.z)

        -- spawn the player
        local ped = PlayerPedId()

        -- V requires setting coords as well
        SetEntityCoordsNoOffset(ped, spawn.x, spawn.y, spawn.z, false, false, false, true)

        NetworkResurrectLocalPlayer(spawn.x, spawn.y, spawn.z, spawn.heading, true, true, false)

        -- gamelogic-style cleanup stuff
        ClearPedTasksImmediately(ped)
        --SetEntityHealth(ped, 300) -- TODO: allow configuration of this?
        RemoveAllPedWeapons(ped) -- TODO: make configurable (V behavior?)
        ClearPlayerWantedLevel(PlayerId())

        -- why is this even a flag?
        --SetCharWillFlyThroughWindscreen(ped, false)

        -- set primary camera heading
        --SetGameCamHeading(spawn.heading)
        --CamRestoreJumpcut(GetGameCam())

        -- load the scene; streaming expects us to do it
        --ForceLoadingScreen(true)
        --loadScene(spawn.x, spawn.y, spawn.z)
        --ForceLoadingScreen(false)

        local time = GetGameTimer()

        while (not HasCollisionLoadedAroundEntity(ped) and (GetGameTimer() - time) < 5000) do
            Citizen.Wait(0)
        end

        ShutdownLoadingScreen()

        if IsScreenFadedOut() then
            DoScreenFadeIn(500)

            while not IsScreenFadedIn() do
                Citizen.Wait(0)
            end
        end

        -- and unfreeze the player
        freezePlayer(PlayerId(), false)

        TriggerEvent('playerSpawned', spawn)

        if cb then
            cb(spawn)
        end

        spawnLock = false
    end)
end

-- automatic spawning monitor thread, too
local respawnForced = true
local diedAt

Citizen.CreateThread(function()
    addSpawnPoint({
        x = 657.723, y = 457.342, z = 144.641,
        heading = 0,
        model = 'a_m_y_skater_01'
    })

    -- main loop thing
    while true do
        Citizen.Wait(50)

        local playerPed = PlayerPedId()

        if playerPed and playerPed ~= -1 then

            -- check if we want to autospawn
            if autoSpawnEnabled then
                if (diedAt and (math.abs(GetTimeDifference(GetGameTimer(), diedAt)) > 2000)) or respawnForced then
                    if autoSpawnCallback then
                        autoSpawnCallback()
                    else
                        spawnPlayer()
                    end

                    respawnForced = false
                end
            end

            if IsEntityDead(playerPed) then
                if not diedAt then
                    diedAt = GetGameTimer()
                end
            else
                diedAt = nil
            end
        end
    end
end)
