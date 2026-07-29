---
ns: CFX
apiset: server
game: gta5
---
## GET_ENTITY_ATTACHMENT_FLAGS

```c
int GET_ENTITY_ATTACHMENT_FLAGS(Entity entity);
```

Gets the attachment flags for the entity, as they were synchronised by the client that owns it.

Returns 0 when the entity is not attached.

## Parameters
* **entity**: The entity handle.

## Return value
The attachment flags.
