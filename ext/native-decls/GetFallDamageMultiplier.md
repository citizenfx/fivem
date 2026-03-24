---
ns: CFX
apiset: client
game: gta5
---
## GET_FALL_DAMAGE_MULTIPLIER

```c
float GET_FALL_DAMAGE_MULTIPLIER();
```

A getter for [SET_FALL_DAMAGE_MULTIPLIER](#_0xF2E1A531).

## Return value
Returns the fall damage multiplier applied to all peds when calculating fall damage from falls **below the kill fall height threshold** (i.e., when the fall does not cause instant death).
The default value is `7.0`.