---
ns: CFX
apiset: server
game: gta5
---
## GET_HELI_ROLL_CONTROL

```c
float GET_HELI_ROLL_CONTROL(Vehicle heli);
```

## Parameters
* **heli**: The helicopter to check.

## Return value
Returns a value representing the roll control of the helicopter. The values range from `-1.0` (roll left) to `1.0` (roll right), with `0.0` indicating no roll input.