---
ns: CFX
apiset: server
---
## GET_VEHICLE_COLOURS

```c
void GET_VEHICLE_COLOURS(Vehicle vehicle, int* colorPrimary, int* colorSecondary);
```


## Parameters
* **vehicle**: 
* **colorPrimary**:
* **colorSecondary**:

## Return value
Return vehicle's color ID.

## Examples
```lua
local veh = GetVehiclePedIsIn(PlayerPedId(), false)
local colorPrimary, colorSecondary = GetVehicleColours(veh)
print(colorPrimary)
print(colorSecondary)
```
