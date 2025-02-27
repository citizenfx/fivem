---
ns: CFX
apiset: server
game: gta5
---
## SET_ENTITY_REMOTE_SYNCED_SCENES_ALLOWED

```c
void SET_ENTITY_REMOTE_SYNCED_SCENES_ALLOWED(Entity entity, bool allow);
```

Enables or disables the owner check for the specified entity in network-synchronized scenes. When set to `false`, the entity cannot participate in synced scenes initiated by clients that do not own the entity.

By default, this is `false` for all entities, meaning only the entity's owner can include it in networked synchronized scenes.

## Parameters
* **entity**: The entity to set the flag for.
* **allow**: Whether to allow remote synced scenes for the entity.
