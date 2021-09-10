---
ns: CFX
apiset: client
---
## GET_VEHICLE_ALARM_TIME_LEFT

```c
int GET_VEHICLE_ALARM_TIME_LEFT(Vehicle vehicle);
```


## Parameters
* **vehicle**: 

Timer left before returning false (in ms)

## Examples
```lua
local veh = GetVehiclePedIsIn(PlayerPedId(), false)
print(GetVehicleAlarmTimeLeft(veh))
```
