---
ns: CFX
apiset: server
---
## GET_ENTITY_MODEL

```c
Hash GET_ENTITY_MODEL(Entity entity);
```

## Parameters
* **entity**: The entity to get the model hash of

## Return value
Returns the models hash, or `0` if the entity doesn't have a model.
