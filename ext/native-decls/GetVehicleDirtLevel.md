---
ns: CFX
apiset: server
---
## GET_VEHICLE_DIRT_LEVEL

```c
float GET_VEHICLE_DIRT_LEVEL(Vehicle vehicle);
```


## Parameters
* **vehicle**: The vehicle to check

## Return value
Return current vehicle dirty level.

## Examples
```lua
local veh = GetVehiclePedIsIn(PlayerPedId(), false)
print(GetVehicleDirtLevel(veh))
```
