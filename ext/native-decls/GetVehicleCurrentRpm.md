---
ns: CFX
apiset: client
---
## GET_VEHICLE_CURRENT_RPM

```c
float GET_VEHICLE_CURRENT_RPM(Vehicle vehicle);
```


## Parameters
* **vehicle**: The vehicle to check.

## Return value
Return current RPM from vehicle on 1.0.

## Examples
```lua
local veh = GetVehiclePedIsIn(PlayerPedId(), false)
print(GetVehicleCurrentRpm(veh))
```
