---
ns: CFX
apiset: server
---
## GET_VEHICLE_HOMING_LOCKON_STATE

```c
int GET_VEHICLE_HOMING_LOCKON_STATE(Vehicle vehicle);
```

Gets the lock on state for the specified vehicle. See the client-side [GET_VEHICLE_HOMING_LOCKON_STATE](#_0xE6B0E8CFC3633BF0) native for a description of lock on states.

## Parameters
* **vehicle**: The vehicle to check.

## Return value
The lock on state.