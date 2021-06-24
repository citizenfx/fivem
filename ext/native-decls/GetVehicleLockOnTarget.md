---
ns: CFX
apiset: server
---
## GET_VEHICLE_LOCK_ON_TARGET

```c
Vehicle GET_VEHICLE_LOCK_ON_TARGET(Vehicle vehicle);
```

Gets the vehicle that is locked on to for the specified vehicle.

## Parameters
* **vehicle**: The vehicle to check.

## Return value
The vehicle that is locked on. 0 returned if no vehicle is locked on.