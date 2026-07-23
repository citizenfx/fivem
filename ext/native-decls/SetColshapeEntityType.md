---
ns: CFX
apiset: shared
---
## SET_COLSHAPE_ENTITY_TYPE

```c
void SET_COLSHAPE_ENTITY_TYPE(int colShapeId, int entityType, BOOL value);
```

Sets whether the collision shape should detect a specific entity type.

By default every entity type is enabled, so a fresh collision shape detects all entities. Setting a type to `false` excludes only that type from detection; every other type stays as it was. To detect a single type (e.g. automobiles only) disable every other type.

The entity type is the sync (network) entity type index (tables below), not the `GET_ENTITY_TYPE` classification.

### GTA5 sync entity types
| Index | Type |
| --- | --- |
| 0 | automobile |
| 1 | bike |
| 2 | boat |
| 3 | door |
| 4 | heli |
| 5 | object |
| 6 | ped |
| 7 | pickup |
| 8 | pickup placement |
| 9 | plane |
| 10 | submarine |
| 11 | player |
| 12 | trailer |
| 13 | train |

### RedM sync entity types
| Index | Type |
| --- | --- |
| 0 | animal |
| 1 | automobile |
| 2 | bike |
| 3 | boat |
| 4 | door |
| 5 | heli |
| 6 | object |
| 7 | ped |
| 8 | pickup |
| 9 | pickup placement |
| 10 | plane |
| 11 | submarine |
| 12 | player |
| 13 | trailer |
| 14 | train |
| 15 | draft vehicle |
| 21 | horse |

## Parameters
* **colShapeId**: The collision shape ID.
* **entityType**: The sync entity type index (see the tables above).
* **value**: `true` to include this entity type in detection, `false` to exclude it.
