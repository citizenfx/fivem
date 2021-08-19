---
ns: CFX
apiset: client
game: gta5
---
## RESET_MAPDATA_ENTITY_MATRIX

```c
BOOL RESET_MAPDATA_ENTITY_MATRIX(int mapDataHash, int entityInternalIdx);
```

Resets mapdata entity transform matrix to its original state.
This function supports SDK infrastructure and is not intended to be used directly from your code.

## Parameters
* **mapDataHash**: A mapdata hash from `mapDataLoaded` event.
* **entityInternalIdx** An internal entity's index.

## Return value
True if successful, false if not.
