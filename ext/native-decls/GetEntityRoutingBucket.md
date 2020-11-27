---
ns: CFX
apiset: server
---
## GET_ENTITY_ROUTING_BUCKET

```c
int GET_ENTITY_ROUTING_BUCKET(Entity entity);
```

Gets the routing bucket for the specified entity.

Routing buckets are also known as 'dimensions' or 'virtual worlds' in past echoes, however they are population-aware.

## Parameters
* **entity**: The entity to get the routing bucket for.

## Return value
The routing bucket ID.
