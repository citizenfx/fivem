---
ns: CFX
apiset: server
game: gta5
---
## GET_HELI_YAW_CONTROL

```c
float GET_HELI_YAW_CONTROL(Vehicle heli);
```

## Parameters
* **heli**: The helicopter to check.

## Return value
Returns a value the yaw control of the helicopter. The value ranges from `-1.0` (yaw left) to `1.0` (yaw right), with `0.0` meaning no yaw input.