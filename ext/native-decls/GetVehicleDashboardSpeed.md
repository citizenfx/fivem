---
ns: CFX
apiset: client
---
## GET_VEHICLE_DASHBOARD_SPEED

```c
float GET_VEHICLE_DASHBOARD_SPEED(Vehicle vehicle);
```


## Parameters
* **vehicle**: The vehicle to check

## Return value
Return current dashbord speed from vehicle.

## Examples
```lua
local veh = GetVehiclePedIsIn(PlayerPedId(), false)
print(GetVehicleDashboardSpeed(veh))
```
