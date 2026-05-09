---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_DASHBOARD_OIL_PRESSURE

```c
float GET_VEHICLE_DASHBOARD_OIL_PRESSURE();
```

The oil pressure will go up/down depending on if the vehicle is moving or sitting still and the engine is on.

If the vehicles engine is off the oil pressure will decrease to `0.0`.

If the vehicle is sitting idle then it will slowly decrease the vehicles oil pressure

## Return value
Returns the vehicles current oil pressure.
