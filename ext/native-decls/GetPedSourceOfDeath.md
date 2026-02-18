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
Returns the last entity id to damage the player, or `0` if the ped hasn't been killed since it respawned.
