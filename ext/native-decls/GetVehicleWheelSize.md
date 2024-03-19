---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_WHEEL_SIZE

```c
float GET_VEHICLE_WHEEL_SIZE(Vehicle vehicle);
```

Returns vehicle's wheels' size (size is the same for all the wheels, cannot get/set specific wheel of vehicle).
Only works on non-default wheels (returns 0 in case of default wheels).

## Parameters
* **vehicle**: The vehicle to obtain data for.

## Return value
Float representing size of the wheel (usually between 0.5 and 1.5)
