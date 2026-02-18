---
ns: CFX
apiset: server
---
## GET_PED_SOURCE_OF_DAMAGE

```c
Entity GET_PED_SOURCE_OF_DAMAGE(Ped ped);
```

Get the last entity that damaged the ped. 

## Parameters
* **ped**: The target ped

## Return value
Returns the last entity id to damage the player, or `0` if the ped hasn't been recently damaged.
