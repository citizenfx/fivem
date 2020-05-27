---
ns: CFX
apiset: client
---
## GET_VEHICLE_WHEEL_RIM_COLLIDER_SIZE

```c
float GET_VEHICLE_WHEEL_RIM_COLLIDER_SIZE(Vehicle vehicle, int wheelIndex);
```


## Parameters
* **vehicle**: The vehicle to obtain data for.
* **wheelIndex**: Index of wheel, 0-3.

## Return value
Float representing size of the rim collider. Not sure what it is used for, probably to detect whether bullets hit rim or tire and puncture it (and to determine size of the wheel when tire is fully blown).