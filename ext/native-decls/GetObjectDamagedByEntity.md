---
ns: CFX
apiset: server
---
## GET_OBJECT_DAMAGED_BY_ENTITY

```c
Entity GET_OBJECT_DAMAGED_BY_ENTITY(Object object);
```

Get the last entity that damaged this object.

## Parameters
* **object**:

## Return value
The entity script guid or 0 if the object has not been damaged.