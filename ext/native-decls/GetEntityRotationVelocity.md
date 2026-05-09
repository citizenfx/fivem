---
ns: CFX
apiset: server
---
## GET_ENTITY_ROTATION_VELOCITY

```c
Vector3 GET_ENTITY_ROTATION_VELOCITY(Entity entity);
```

**NOTE**: This will not work for peds, it will return `vector3(0, 0, 0)`, this is because ped velocity is simulated entirely on the client.

## Parameters
* **entity**: The entity to get the rotation velocity of

## Return value
Returns the entities current rotation velocity
