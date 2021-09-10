---
ns: CFX
apiset: client
---
## GET_VEHICLE_CLUTCH

```c
float GET_VEHICLE_CLUTCH(Vehicle vehicle);
```


## Parameters
* **vehicle**: The target vehicle.

## Return value
Returns vehicle's clutch system.


## Examples
```lua
local veh = GetVehiclePedIsIn(PlayerPedId(), false)
print(GetVehicleClutch(veh))
```
