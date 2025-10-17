---
ns: CFX
apiset: server
---
## IS_PLAYER_ACE_ALLOWED

```c
BOOL IS_PLAYER_ACE_ALLOWED(char* playerSrc, char* aceIdentifier);
```

For a more detailed example please [refer to this guide](https://forum.cfx.re/t/basic-aces-principals-overview-guide/90917).

## Parameters
* **playerSrc**: The player to check
* **aceIdentifier**: The ace to check, i.e. `command.ensure`

## Return value
Returns `true` if the ped has permission for the specified ace, `false` otherwise
