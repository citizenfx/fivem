---
ns: CFX
apiset: server
---
## GET_ENTITY_ATTACHED_TO

```c
Entity GET_ENTITY_ATTACHED_TO(Entity entity);
```

Gets the entity that `entity` is attached to.

## Parameters
* **entity**: The entity to check.

## Return value
Returns the attached entity handle, or `0` if the entity isn't attached to anything.
