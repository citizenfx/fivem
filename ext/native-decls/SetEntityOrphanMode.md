---
ns: CFX
apiset: server
---
## SET_ENTITY_ORPHAN_MODE

```c
void SET_ENTITY_ORPHAN_MODE(Entity entity, int orphanMode);
```

```c
enum EntityOrphanMode {
    // Default, this will delete the entity when it isn't relevant to any players
    // NOTE: this *doesn't* mean when they're no longer in scope
    DeleteWhenNotRelevant = 0,
    // The entity will be deleted whenever its original owner disconnects
    // NOTE: if this is set when the entities original owner has already left it will be
    // marked for deletion (similar to just calling DELETE_ENTITY)
    DeleteOnOwnerDisconnect = 1,
    // The entity will never be deleted by the server when it does relevancy checks
    // you should only use this on entities that need to be relatively persistent
    KeepEntity = 2
}
```

Sets what the server will do when the entity no longer has its original owner. By default the server will cleanup entities that it considers "no longer relevant".

When used on trains, this native will recursively call onto all attached carriages.

**NOTE**: When used with `KeepEntity` (2) this native only guarantees that the ***server*** will not delete the entity, client requests to delete the entity will still work perfectly fine.

## Parameters
* **entity**: The entity to set the orphan mode of
* **orphanMode**: The mode that the server should use for determining if an entity should be removed.
