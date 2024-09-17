---
ns: CFX
apiset: client
game: gta5
---
## OVERRIDE_PEDS_USE_DEFAULT_DRIVE_BY_CLIPSET

```c
void OVERRIDE_PEDS_USE_DEFAULT_DRIVE_BY_CLIPSET(BOOL flag);
```

Allows the bypassing of default game behavior that prevents the use of [SET_PED_DRIVE_BY_CLIPSET_OVERRIDE](#_0xED34AB6C5CB36520) in certain scenarios to avoid clipping issues (e.g., when there is more than one Ped in a vehicle).

Note: This flag and the overridden clipset are not replicated values and require synchronization through user scripts. Additionally, current game behavior also restricts applying this clipset locally when in first-person mode and will require a temporary workaround.

## Parameters
* **flag**: true to override, false to use default game behavior.
