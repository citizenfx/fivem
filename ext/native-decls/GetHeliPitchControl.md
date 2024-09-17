---
ns: CFX
apiset: server
game: gta5
---
## GET_HELI_PITCH_CONTROL

```c
float GET_HELI_PITCH_CONTROL(Vehicle heli);
```

## Parameters
* **heli**: The helicopter to check.

## Return value
Returns a value representing the pitch control of the helicopter. The values range from `-1.0` (nose down) to `1.0` (nose up), with `0.0` indicating no pitch input.