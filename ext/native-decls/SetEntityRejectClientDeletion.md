---
ns: CFX
apiset: server
---
## SET_ENTITY_REJECTS_CLIENT_DELETION

```c
void SET_ENTITY_REJECTS_CLIENT_DELETION(Entity entity, bool rejectClientDelete);
```

This native only works on server-created vehicles.

Sets a flag for the server to reject any client side deletion of this entity.

This currently *doesn't* stop the entity from being deleted on the calling client, it will just recreate the entity for the caller and reset the relevant state on the server.

This flag will default to the value of "sv_disallowClientDelete" when the entity was created

## Parameters
* **entity**: The entity to reject deletion of
* **rejectClientDelete**: A boolean of whether the server should reject client delete, or accept it
