---
ns: CFX
apiset: server
---
## DOES_PLAYER_OWN_SKU

```c
BOOL DOES_PLAYER_OWN_SKU(char* playerSrc, int skuId);
```

Requests whether or not the player owns the specified SKU.

## Parameters
* **playerSrc**: The player handle
* **skuId**: The ID of the SKU.

## Return value
A boolean.