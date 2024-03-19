---
ns: CFX
apiset: client
game: gta5
---
## SET_VEHICLE_WHEEL_WIDTH

```c
BOOL SET_VEHICLE_WHEEL_WIDTH(Vehicle vehicle, float width);
```

Sets vehicle's wheels' width (width is the same for all the wheels, cannot get/set specific wheel of vehicle).
Only works on non-default wheels.
Returns whether change was successful (can be false if trying to set width for non-default wheels).

## Parameters
* **vehicle**: The vehicle to set data for.
* **width**: Width of the wheels (usually between 0.1 and 1.5 is reasonable).

## Return value
Bool - whether change was successful or not
