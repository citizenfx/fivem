---
ns: CFX
apiset: client
game: gta5
---
## GENERATE_PEDS_AT_SCENARIO_POINTS

```c
void GENERATE_PEDS_AT_SCENARIO_POINTS(float popControlCentreX, float popControlCentreY, float popControlCentreZ, BOOL allowDeepInteriors, float rangeOutOfViewMin, float  rangeOutOfViewMax, float rangeInViewMin, float rangeInViewMax, float rangeFrustumExtra, bool doInFrustumTest, int maxPeds, int maxInteriorPeds);
```

Generates peds for scenario points in the provided view range, optionally include / exclude interiors.
this operation is quite costly, so try not to call it to frequently.

## Parameters
* **popControlCentreX**: X position of the control center.
* **popControlCentreY**: Y position of the control center.
* **popControlCentreZ**: Z position of the control center.
* **allowDeepInteriors**: Allows peds to generate deep inside interiors.
* **rangeOutOfViewMin**: Min range at which peds generate outside the players view, keep within ~10 meters of max value.
* **rangeOutOfViewMax**: Max range at which peds generate outside the players view, keep within ~10 meters of min value.
* **rangeInViewMin**: Min range at which peds generate inside the players view, keep within ~10 meters of max value.
* **rangeInViewMax**: Max range at which peds generate inside the players view, keep within ~10 meters of min value.
* **rangeFrustumExtra**: extends frustum distance.
* **doInFrustumTest**: if true, check if the scenario point can be seen by the player.
* **maxPeds**: Maximum amount of peds to generate.
* **maxInteriorPeds**: Maximum amount of interior peds to generate.

## Examples
```lua
Citizen.CreateThread(function()
    while true do
        Citizen.Wait(5000) -- generate peds every 5 seconds
        local playerPed = PlayerPedId()
        local pos = GetEntityCoords(playerPed)
        local x, y, z = table.unpack(pos)
        local allowDeepInteriors = false
        local rangeOutOfViewMin = 10.0
        local rangeOutOfViewMax = 20.0
        local rangeInViewMin = 20.0
        local rangeInViewMax = 30.0
        local rangeFrustumExtra = 50.0
        local doInFrustumTest = false
        local maxPeds = 100
        local maxInteriorPeds = 100
        GeneratePedsAtScenarioPoints(x, y, z, allowDeepInteriors, rangeOutOfViewMin, rangeOutOfViewMax, rangeInViewMin, rangeInViewMax,
                                    rangeFrustumExtra, doInFrustumTest, maxPeds, maxInteriorPeds)
    end
end, false)
```