---
ns: CFX
apiset: client
---
## GET_VEHICLE_OIL_LEVEL

```c
// 0xFC7F8EF4
float GET_VEHICLE_OIL_LEVEL(Vehicle vehicle);
```


## Parameters
* **vehicle**: 

## Return value
Return a value between 0.0 and 10.0 representing the current oil level.

## Examples
```lua
-- A short example of how to print value of oil level in Lua
local vehicle = GetVehiclePedIsIn(PlayerPedId(), false)
local oilLvl = GetVehicleOilLevel(vehicle)
print(oilLvl)
```
