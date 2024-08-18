---
ns: CFX
apiset: server
---
## GET_ENTITY_HEALTH

```c
int GET_ENTITY_HEALTH(Entity entity);
```

Only works for vehicle and peds

## Parameters
* **entity**: The entity to check the health of

## Return value
If the entity is a vehicle it will return 0-1000
If the entity is a ped it will return 0-200
If the entity is an object it will return 0
