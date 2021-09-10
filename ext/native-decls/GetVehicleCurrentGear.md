---
ns: CFX
apiset: client
---
## GET_VEHICLE_CURRENT_GEAR

```c
int GET_VEHICLE_CURRENT_GEAR(Vehicle vehicle);
```


## Parameters
* **vehicle**: The vehicle to check.

## Return value
Return current gear value from vehicle

## Examples
```lua
local veh = GetVehiclePedIsIn(PlayerPedId(), false)
print(GetVehicleCurrentGear(veh))
```
