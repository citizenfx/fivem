---
ns: CFX
apiset: client
---
## SET_VEHICLE_OIL_LEVEL
```c
void SET_VEHICLE_OIL_LEVEL(Vehicle vehicle, float level);
```

Setting the oil level value. This native does not affect vehicle performance.

## Parameters
* **vehicle**: 
* **level**: Recommended value between 0.0 and 10.0 maximum (used by default in game).

## Examples
```lua
-- A short example setting the oil level value in Lua
local vehicle = GetVehiclePedIsIn(PlayerPedId(), false)
SetVehicleOilLevel(vehicle, 10.0)
```

