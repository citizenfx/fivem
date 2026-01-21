---
ns: CFX
apiset: server
---
## GET_ENTITY_VELOCITY

```c
Vector3 GET_ENTITY_VELOCITY(Entity entity);
```

**NOTE**: This will not work for peds, it will return `vector3(0, 0, 0)`, this is because ped velocity is simulated entirely on the client.

## Parameters
* **entity**: The entity to get the velocity of

## Return value
Returns the entities current velocity
