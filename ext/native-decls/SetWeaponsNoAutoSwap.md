---
ns: CFX
apiset: client
---
## SET_WEAPONS_NO_AUTOSWAP

```c
void SET_WEAPONS_NO_AUTOSWAP(BOOL state);
```

Disables autoswapping to another weapon when the current weapon runs out of ammo.

## Parameters
* **state**: On/Off

```
This native won't have any affect if setting flag 48 to true, using SetPedConfigFlag native.
```
