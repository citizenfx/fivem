---
ns: CFX
apiset: client
---
## GET_VEHICLE_HAS_FLAG

```c
bool GET_VEHICLE_HAS_FLAG(Vehicle vehicle, int flagIndex);
```

Get vehicle.meta flag by index. Useful examples include FLAG_LAW_ENFORCEMENT (31), FLAG_RICH_CAR (36), FLAG_IS_ELECTRIC (43), FLAG_IS_OFFROAD_VEHICLE (48).

For a complete list of flags, you can refer to the [Vehicle Flags documentation](https://docs.fivem.net/docs/game-references/vehicle-references/vehicle-flags/).

## Parameters
* **vehicle**: The vehicle to obtain data for.
* **flagIndex**: Flag index (0-203)

## Return value
A boolean for whether the flag is set.

## Examples
```lua
local function getVehicleFlags(vehicle)
    local vehicleFlags = {}
    for i = 0, 203 do
        if GetVehicleHasFlag(vehicle, i) then
            vehicleFlags[#vehicleFlags+1] = i
        end
    end
    return vehicleFlags
end

local vehicle = GetVehiclePedIsIn(PlayerPedId(), false)
getVehicleFlags(vehicle)
```

```js
function getVehicleFlags(vehicle) {
    const vehicleFlags = [];
    for (let i = 0; i <= 203; i++) {
        if (GetVehicleHasFlag(vehicle, i)) {
            vehicleFlags.push(i);
        }
    }
    return vehicleFlags;
}

const vehicle = GetVehiclePedIsIn(PlayerPedId(), false);
getVehicleFlags(vehicle);
```