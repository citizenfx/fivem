---
ns: CFX
apiset: server
---
## SET_ENTITY_ROUTING_BUCKET

```c
void SET_ENTITY_ROUTING_BUCKET(Entity entity, int bucket);
```

Sets the routing bucket for the specified entity.

Routing buckets are also known as 'dimensions' or 'virtual worlds' in past echoes, however they are population-aware.

## Parameters
* **entity**: The entity to set the routing bucket for.
* **bucket**: The bucket ID.
