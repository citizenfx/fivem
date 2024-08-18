---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_WHEEL_SUSPENSION_COMPRESSION

```c
float GET_VEHICLE_WHEEL_SUSPENSION_COMPRESSION(Vehicle vehicle, int wheelIndex);
```

Gets the current suspension compression of a wheel.
Returns a positive value. 0 means the suspension is fully extended, the wheel is off the ground.
Max number of wheels can be retrieved with the native GET_VEHICLE_NUMBER_OF_WHEELS.

## Parameters
* **vehicle**:
* **wheelIndex**:

## Return value
The current compression of the wheel's suspension.