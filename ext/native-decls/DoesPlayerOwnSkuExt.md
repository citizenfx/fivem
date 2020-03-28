---
ns: CFX
apiset: server
---
## DOES_PLAYER_OWN_SKU_EXT

```c
BOOL DOES_PLAYER_OWN_SKU_EXT(char* playerSrc, int skuId);
```

Requests whether or not the player owns the specified package.

## Parameters
* **playerSrc**: The player handle
* **skuId**: The package ID on Tebex.

## Return value
A boolean.