---
ns: CFX
apiset: server
---
## SET_ROUTING_BUCKET_POPULATION_ENABLED

```c
void SET_ROUTING_BUCKET_POPULATION_ENABLED(int bucketId, BOOL mode);
```

Sets whether or not the specified routing bucket has automatically-created population enabled.

## Parameters
* **bucketId**: The routing bucket ID to adjust.
* **mode**: `true` to enable population, `false` to disable population.
