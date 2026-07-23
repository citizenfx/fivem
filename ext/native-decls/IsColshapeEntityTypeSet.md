---
ns: CFX
apiset: shared
---
## IS_COLSHAPE_ENTITY_TYPE_SET

```c
BOOL IS_COLSHAPE_ENTITY_TYPE_SET(int colShapeId, int entityType);
```

Returns whether the collision shape is set to detect the given entity type. Every type is enabled by default; a type reads as disabled only after `SET_COLSHAPE_ENTITY_TYPE(colShapeId, entityType, false)`.

## Parameters
* **colShapeId**: The collision shape ID.
* **entityType**: The sync (network) entity type index (e.g. GTA5: automobile `0`, object `5`, ped `6`, player `11`). See `SET_COLSHAPE_ENTITY_TYPE` for the full per-game tables.

## Return value
Returns true if the entity type is detected by the collision shape, false otherwise.
