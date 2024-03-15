---
ns: CFX
apiset: client
game: gta5
---
## SET_VEHICLE_WHEEL_SIZE

```c
BOOL SET_VEHICLE_WHEEL_SIZE(Vehicle vehicle, float size);
```

Sets vehicle's wheels' size (size is the same for all the wheels, cannot get/set specific wheel of vehicle).
Only works on non-default wheels.
Returns whether change was successful (can be false if trying to set size for non-default wheels).

## Parameters
* **vehicle**: The vehicle to set data for.
* **size**: Size of the wheels (usually between 0.5 and 1.5 is reasonable).

## Return value
Bool - whether change was successful or not
