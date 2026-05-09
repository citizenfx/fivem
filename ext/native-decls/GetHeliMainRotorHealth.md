---
ns: CFX
apiset: server
---
## GET_HELI_MAIN_ROTOR_HEALTH

```c
float GET_HELI_MAIN_ROTOR_HEALTH(Vehicle vehicle);
```

## Parameters
* **vehicle**: The target vehicle.

## Return value
Returns a value between `1000.0` (full health) to `0.0` (rotor broken)
