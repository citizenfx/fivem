---
ns: CFX
apiset: server
---
## LOAD_PLAYER_COMMERCE_DATA_EXT

```c
void LOAD_PLAYER_COMMERCE_DATA_EXT(char* playerSrc);
```

Requests the commerce data from Tebex for the specified player, including the owned SKUs. Use `IS_PLAYER_COMMERCE_INFO_LOADED` to check if it has loaded.

## Parameters
* **playerSrc**: The player handle

