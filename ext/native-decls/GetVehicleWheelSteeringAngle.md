---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_WHEEL_STEERING_ANGLE

```c
float GET_VEHICLE_WHEEL_STEERING_ANGLE(Vehicle vehicle, int wheelIndex);
```

Gets steering angle of a wheel.
Max number of wheels can be retrieved with the native GET_VEHICLE_NUMBER_OF_WHEELS.

## Parameters
* **vehicle**:
* **wheelIndex**:

## Return value
The steering angle of the wheel, with 0 being straight.
