---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_WHEEL_ROTATION_SPEED

```c
float GET_VEHICLE_WHEEL_ROTATION_SPEED(Vehicle vehicle, int wheelIndex);
```

Gets the rotation speed of a wheel.
This is used internally to calcuate GET_VEHICLE_WHEEL_SPEED.
Max number of wheels can be retrieved with the native GET_VEHICLE_NUMBER_OF_WHEELS.

## Parameters
* **vehicle**:
* **wheelIndex**:

## Return value
The angular velocity of the wheel.