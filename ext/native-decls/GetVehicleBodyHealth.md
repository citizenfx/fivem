---
ns: CFX
apiset: server
---
## GET_VEHICLE_BODY_HEALTH

```c
float GET_VEHICLE_BODY_HEALTH(Vehicle vehicle);
```


## Parameters
* **vehicle**: 

## Return value
Vehicle condition on a value of 1000.0

## Examples
```lua
local veh = GetVehiclePedIsIn(PlayerPedId(), false)
print(GetVehicleBodyHealth(veh))
```
