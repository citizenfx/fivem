---
ns: CFX
apiset: server
---
## GET_ENTITY_ATTACHED_TO

```c
Entity GET_ENTITY_ATTACHED_TO(Entity entity);
```

Gets the entity that this entity is attached to.

## Parameters
* **entity**: The entity to check.

## Return value
The attached entity handle. 0 returned if the entity is not attached.