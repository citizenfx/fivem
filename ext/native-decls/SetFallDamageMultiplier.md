---
ns: CFX
apiset: client
game: gta5
---
## SET_FALL_DAMAGE_MULTIPLIER

```c
void SET_FALL_DAMAGE_MULTIPLIER(float multiplier);
```

A setter for [GET_FALL_DAMAGE_MULTIPLIER](#_0x2D6A0A83).

## Parameters
* **multiplier**: fall damage multiplier applied to all peds when calculating fall damage from falls below the kill fall height threshold (i.e., when the fall does not cause instant death). Default value is `7.0`.