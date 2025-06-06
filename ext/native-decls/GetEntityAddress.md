---
ns: CFX
apiset: client
---
## GET_ENTITY_ADDRESS

```c
Any* GET_ENTITY_ADDRESS(Entity entity);
```

**Experimental**: This native may be altered or removed in future versions of CitizenFX without warning.

Returns the memory address of an entity.

This native is intended for singleplayer debugging, and may not be available during multiplayer.

## Parameters
* **entity**: The handle of the entity to get the address of.

## Return value
A pointer containing the memory address of the entity.