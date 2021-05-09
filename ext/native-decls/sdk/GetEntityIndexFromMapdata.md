---
ns: CFX
apiset: client
game: gta5
---
## GET_ENTITY_INDEX_FROM_MAPDATA

```c
int GET_ENTITY_INDEX_FROM_MAPDATA(int mapdata, int entity);
```

Returns the transient entity index for a specified mapdata/entity pair.
This function supports SDK infrastructure and is not intended to be used directly from your code.

## Parameters
* **mapdata**: The input map data index from GET_MAPDATA_FROM_HASH_KEY.
* **entity**: The input entity handle from GET_ENTITY_MAPDATA_OWNER.

## Return value
A transient (non-persistable) index to the requested entity, or -1.
