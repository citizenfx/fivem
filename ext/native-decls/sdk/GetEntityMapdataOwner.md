---
ns: CFX
apiset: client
game: gta5
---
## GET_ENTITY_MAPDATA_OWNER

```c
BOOL GET_ENTITY_MAPDATA_OWNER(Entity entity, int* mapdataHandle, int* entityHandle);
```

Retrieves the map data and entity handles from a specific entity.
This function supports SDK infrastructure and is not intended to be used directly from your code.

## Parameters
* **entity**: An entity owned by map data.
* **mapdataHandle**: The output map data handle.
* **entityHandle**: The output entity handle.

## Return value
True if successful, false if not.
