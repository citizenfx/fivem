---
ns: CFX
apiset: server
---
## GET_VEHICLE_DASHBOARD_COLOUR

```c
void GET_VEHICLE_DASHBOARD_COLOUR(Vehicle vehicle, int* color);
```


## Parameters
* **vehicle**: The vehicle to check.
* **color**: Color ID.

## Return value
Return vehicle's dashbord color ID.

## Examples
```lua
local veh = GetVehiclePedIsIn(PlayerPedId(), false)
print(GetVehicleDashboardColour(veh))
```
