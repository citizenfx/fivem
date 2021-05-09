---
ns: CFX
apiset: client
game: gta5
---
## GET_MAPDATA_FROM_HASH_KEY

```c
int GET_MAPDATA_FROM_HASH_KEY(Hash mapdataHandle);
```

Returns the transient map data index for a specified hash.
This function supports SDK infrastructure and is not intended to be used directly from your code.

## Parameters
* **mapdataHandle**: The input map data handle.

## Return value
A transient (non-persistable) index to the requested mapdata, or -1.