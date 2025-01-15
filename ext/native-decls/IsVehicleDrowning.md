---
ns: CFX
apiset: server
---
## IS_VEHICLE_DROWNING

```c
bool IS_VEHICLE_DROWNING(Vehicle vehicle);
```

Getter to check if the current vehicle is drowning.

## Parameters
* **vehicle**: The target vehicle.

## Return value
Return `true` if the vehicle is drowning (and the vehicle is underwater), `false` otherwise.