---
ns: CFX
apiset: server
game: gta5
---
## GET_ENTITY_ATTACHMENT_OFFSET

```c
Vector3 GET_ENTITY_ATTACHMENT_OFFSET(Entity entity);
```

Gets the offset the entity is attached at, relative to the entity it is attached to.

Returns a zero vector when the entity is not attached, or when it is attached without an offset.

## Parameters
* **entity**: The entity handle.

## Return value
The attachment offset.
