---
ns: CFX
apiset: server
game: gta5
---
## GET_HELI_THROTTLE_CONTROL

```c
float GET_HELI_THROTTLE_CONTROL(Vehicle heli);
```

## Parameters
* **heli**: The helicopter to check.

## Return value
Returns a value representing the throttle control of the helicopter. The value ranges from `0.0` (no throttle) to `2.0` (full throttle).