---
ns: CFX
apiset: client
---
## GET_VEHICLE_HAS_FLAG

```c
bool GET_VEHICLE_HAS_FLAG(Vehicle vehicle, int flagIndex);
```

**Note**: Flags are not the same based on your `gamebuild`. Please see [here](https://docs.fivem.net/docs/game-references/vehicle-references/vehicle-flags) to see a complete list of all vehicle flags.

Get vehicle.meta flag by index. Useful examples include `FLAG_LAW_ENFORCEMENT` (31), `FLAG_RICH_CAR` (36), `FLAG_IS_ELECTRIC` (43), `FLAG_IS_OFFROAD_VEHICLE` (48).

## Parameters
* **vehicle**: The vehicle to obtain flags for.
* **flagIndex**: Flag index.

## Return value
A boolean for whether the flag is set.

### Example
```lua
local vehicleFlags = {
    FLAG_SMALL_WORKER = 0,
    FLAG_BIG = 1,
    FLAG_NO_BOOT = 2,
    FLAG_ONLY_DURING_OFFICE_HOURS = 3
    -- This is just a example, see fivem-docs to see all flags.
}

local function getAllVehicleFlags(vehicle)
    local flags = {}
    for i = 0, 256 do
        if GetVehicleHasFlag(vehicle, i) then
            flags[#flags+1] = i
        end
    end
    return flags
end

local flagsVehicle = GetVehiclePedIsIn(PlayerPedId(), false)
print(getAllVehicleFlags)
```

```javascript
const VEHICLE_FLAGS = {
    FLAG_SMALL_WORKER: 0,
    FLAG_BIG: 1,
    FLAG_NO_BOOT: 2,
    FLAG_ONLY_DURING_OFFICE_HOURS: 3
    // This is just a example, see fivem-docs to see all flags.
};

function getAllVehicleFlags(mVehicle = GetVehiclePedIsIn(PlayerPedId(), false)) {
    const flags = [];
    for (let i = 0; i < 204; i++) {
        if (GetVehicleHasFlag(mVehicle, i)) {
            flags.push(i);
        }
    }
    return flags;
}

let flagsVehicle = GetVehiclePedIsIn(PlayerPedId(), false);
console.log(getAllVehicleFlags);
```