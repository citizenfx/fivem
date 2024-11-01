---
ns: CFX
apiset: server
---
## DELETE_ENTITY

```c
void DELETE_ENTITY(Entity entity);
```

Deletes the specified entity.

**NOTE**: For trains this will only work if called on the train engine, it will not work on its carriages.

## Parameters
* **entity**: The entity to delete.
