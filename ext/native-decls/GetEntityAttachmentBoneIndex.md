---
ns: CFX
apiset: server
game: gta5
---
## GET_ENTITY_ATTACHMENT_BONE_INDEX

```c
int GET_ENTITY_ATTACHMENT_BONE_INDEX(Entity entity);
```

Gets the bone index of the entity the given entity is attached to.

## Parameters
* **entity**: The entity handle.

## Return value
The bone index, or -1 when the entity is not attached or is not attached to a bone.
