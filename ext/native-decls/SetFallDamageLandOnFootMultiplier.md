---
ns: CFX
apiset: client
game: gta5
---
## SET_FALL_DAMAGE_LAND_ON_FOOT_MULTIPLIER

```c
void SET_FALL_DAMAGE_LAND_ON_FOOT_MULTIPLIER(float multiplier);
```

A setter for [GET_FALL_DAMAGE_LAND_ON_FOOT_MULTIPLIER](#_0x3C8A1C92).

## Parameters
* **multiplier**: fall damage multiplier to apply when a ped lands on foot from a fall below the kill fall height threshold (i.e., when the fall does not cause instant death). Default value is `3.0`.