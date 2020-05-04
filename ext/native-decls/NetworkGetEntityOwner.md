---
ns: CFX
apiset: shared
---
## NETWORK_GET_ENTITY_OWNER

```c
int NETWORK_GET_ENTITY_OWNER(Entity entity);
```

Returns the owner ID of the specified entity.

## Parameters
* **entity**: The entity to get the owner for.

## Return value
On the server, the server ID of the entity owner. On the client, returns the player/slot ID of the entity owner.