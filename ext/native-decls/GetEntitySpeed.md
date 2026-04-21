---
ns: CFX
apiset: server
---
## GET_ENTITY_SPEED

```c
float GET_ENTITY_SPEED(Entity entity);
```

Gets the current speed of the entity in meters per second.

**NOTE**: This will not work for peds, it will return `0.0`, this is because ped velocity is simulated entirely on the client.

```
To convert to MPH: speed * 2.236936
To convert to KPH: speed * 3.6
```

## Parameters
* **entity**: The entity to get the speed of

## Return value
The speed of the entity in meters per second
