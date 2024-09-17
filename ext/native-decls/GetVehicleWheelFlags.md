---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_WHEEL_FLAGS

```c
int GET_VEHICLE_WHEEL_FLAGS(Vehicle vehicle, int wheelIndex);
```

Gets the flags of a wheel.
Max number of wheels can be retrieved with the native GET_VEHICLE_NUMBER_OF_WHEELS.

## Parameters
* **vehicle**:
* **wheelIndex**:

## Return value
An unsigned int containing bit flags.