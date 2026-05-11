---
ns: CFX
apiset: server
---
## LOAD_PLAYER_COMMERCE_DATA

```c
void LOAD_PLAYER_COMMERCE_DATA(char* playerSrc);
```

> This native is deprecated and may be removed in a future version. Use [`LOAD_PLAYER_COMMERCE_DATA_EXT`](#_0x7995539E) instead.

Requests the commerce data for the specified player, including the owned SKUs. Use `IS_PLAYER_COMMERCE_INFO_LOADED` to check if it has loaded.

## Parameters
* **playerSrc**: The player handle

