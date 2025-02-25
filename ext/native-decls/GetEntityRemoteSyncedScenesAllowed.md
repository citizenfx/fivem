---
ns: CFX
apiset: server
game: gta5
---
## GET_ENTITY_REMOTE_SYNCED_SCENES_ALLOWED

```c
BOOL GET_ENTITY_REMOTE_SYNCED_SCENES_ALLOWED(Entity entity);
```

## Parameters
* **entity**: The entity to get the flag for.

## Return value
Returns if the entity is allowed to participate in network-synchronized scenes initiated by clients that do not own the entity.
