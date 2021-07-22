---
ns: CFX
apiset: client
game: gta5
---
## GET_MAPDATA_ENTITY_HANDLE

```c
BOOL GET_MAPDATA_ENTITY_HANDLE(int mapDataHash, int entityInternalIdx, int* entityHandle);
```

Retrieves the map data entity handle.
This function supports SDK infrastructure and is not intended to be used directly from your code.

## Parameters
* **mapDataHash**: A mapdata hash from `mapDataLoaded` event.
* **entityInternalIdx** An internal entity's index.
* **entityHandle**: The output entity handle.

## Return value
True if successful, false if not.
