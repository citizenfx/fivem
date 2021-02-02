---
ns: CFX
apiset: server
---
## GET_PED_SOURCE_OF_DAMAGE

```c
Entity GET_PED_SOURCE_OF_DAMAGE(Ped ped);
```

Get the last entity that damaged the ped. This native is used server side when using OneSync.

## Parameters
* **ped**: The target ped

## Return value
The entity id. Returns 0 if the ped has not been damaged recently.
