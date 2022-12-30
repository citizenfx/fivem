---
ns: CFX
apiset: server
---

## GET_PLAYER_SERVER_ID_FROM_PED 

```c
int GET_PLAYER_SERVER_ID_FROM_PED(Ped ped);
```

## Parameters
- **ped**: The ped to get the server id of.

## Return value
Returns the players server id, or 0 if the ped didn't exist, or if it wasn't a player ped
