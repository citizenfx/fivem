---
ns: CFX
apiset: server
game: gta5
---
## GET_HELI_TAIL_ROTOR_DAMAGE_SCALE

```c
float GET_HELI_TAIL_ROTOR_DAMAGE_SCALE(Vehicle heli);
```

## Parameters
* **heli**: The helicopter to check

## Return value
Returns a value representing the damage scaling factor applied to the helicopter's tail rotor. The value ranges from `0.0` (no damage scaling) to` 1.0` (full damage scaling).