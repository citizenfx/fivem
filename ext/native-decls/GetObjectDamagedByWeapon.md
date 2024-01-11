---
ns: CFX
apiset: server
---
## GET_OBJECT_DAMAGED_BY_WEAPON

```c
Hash GET_OBJECT_DAMAGED_BY_WEAPON(Object object);
```

Gets the weapon hash that last damaged this object.

## Parameters
* **object**:

## Return value
The weapon hash or 0 if the object has not been damaged.