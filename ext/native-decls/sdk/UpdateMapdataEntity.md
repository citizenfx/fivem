---
ns: CFX
apiset: client
game: gta5
---
## UPDATE_MAPDATA_ENTITY

```c
void UPDATE_MAPDATA_ENTITY(int mapdata, int entity, object entityDef);
```

Transiently updates the entity with the specified mapdata index and entity index.
This function supports SDK infrastructure and is not intended to be used directly from your code.

## Parameters
* **mapdata**: A fwMapData index from GET_MAPDATA_FROM_HASH_KEY.
* **entity**: An entity index from GET_ENTITY_INDEX_FROM_MAPDATA.
* **entityDef**: The new entity definition in fwEntityDef format.