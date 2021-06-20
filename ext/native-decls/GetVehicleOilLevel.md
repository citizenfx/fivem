---
ns: CFX
apiset: client
---
## GET_VEHICLE_OIL_LEVEL

```c
float GET_VEHICLE_OIL_LEVEL(Vehicle vehicle);
```


## Parameters
* **vehicle**: 

## Return value
Return a value between 0.0 and 10.0 representing the current oil level.

## Examples
```lua
-- A short example printing the oil level value in Lua
local vehicle = GetVehiclePedIsIn(PlayerPedId(), false)
local oilLvl = GetVehicleOilLevel(vehicle)
print(oilLvl)
```
