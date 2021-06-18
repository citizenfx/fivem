---
ns: CFX
apiset: client
---
## SET_VEHICLE_OIL_LEVEL

```c
void SET_VEHICLE_OIL_LEVEL(Vehicle vehicle, float level);
```

Adjust the vehicle's oil value. This native does not affect vehicle performance.

## Parameters
* **vehicle**: 
* **level**: Recommended value between 0.0 and 10.0 maximum (used by default in game).

## Examples
```lua
-- A short example of how to set a value for oil level in Lua
local vehicle = GetVehiclePedIsIn(PlayerPedId(), false)
SetVehicleOilLevel(vehicle, 10.0)
```
