---
ns: CFX
apiset: client
game: gta5
---
## DELETE_SCREEN_PED

```c
void DELETE_SCREEN_PED(Ped ped);
```

Removes a screen ped created with `CLONE_PED_TO_SCREEN` from the screen and deletes the cloned ped. Screen peds are also removed automatically when the owning resource stops.

## Parameters
* **ped**: The screen ped handle returned by `CLONE_PED_TO_SCREEN`.

## Return value
None.
