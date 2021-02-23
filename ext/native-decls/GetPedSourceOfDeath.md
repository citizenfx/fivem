---
ns: CFX
apiset: server
---
## GET_PED_SOURCE_OF_DEATH

```c
Entity GET_PED_SOURCE_OF_DEATH(Ped ped);
```

Get the entity that killed the ped. This native is used server side when using OneSync.

## Parameters
* **ped**: The target ped

## Return value
The entity id. Returns 0 if the ped has no killer.
