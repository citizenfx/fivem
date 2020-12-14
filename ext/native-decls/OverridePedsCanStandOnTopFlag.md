---
ns: CFX
apiset: client
game: gta5
---
## OVERRIDE_PEDS_CAN_STAND_ON_TOP_FLAG

```c
void OVERRIDE_PEDS_CAN_STAND_ON_TOP_FLAG(BOOL flag);
```

Sets whether peds can stand on top of *all* vehicles without falling off.

Note this flag is not replicated automatically, you will have to manually do so.

## Parameters
* **flag**: true to override, false to use default game behavior.