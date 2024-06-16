---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_WHEEL_IS_POWERED

```c
BOOL GET_VEHICLE_WHEEL_IS_POWERED(Vehicle vehicle, int wheelIndex);
```

Gets whether the wheel is powered.
Max number of wheels can be retrieved with the native GET_VEHICLE_NUMBER_OF_WHEELS.
This is a shortcut to a flag in GET_VEHICLE_WHEEL_FLAGS.

## Parameters
* **vehicle**:
* **wheelIndex**:
