---
ns: CFX
apiset: shared
---
## IS_PRINCIPAL_ACE_ALLOWED

```c
BOOL IS_PRINCIPAL_ACE_ALLOWED(char* principal, char* aceIdentifier);
```

For a more detailed example please [refer to this guide](https://forum.cfx.re/t/basic-aces-principals-overview-guide/90917).

## Parameters
* **principal**: The principal to check, i.e. `group.admin`
* **aceIdentifier**: The ace to check, i.e. `command.ensure`

## Return value
Returns `true` if the specified `principal` has permissions to use `aceIdentifier`
