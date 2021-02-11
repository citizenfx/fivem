---
ns: CFX
apiset: server
---
## GET_PLAYER_WANTED_CENTRE_POSITION

```c
Vector3 GET_PLAYER_WANTED_CENTRE_POSITION(char* playerSrc);
```

Gets the current known coordinates for the specified player from cops perspective. This native is used server side when using OneSync.

## Parameters
* **playerSrc**: The target player

## Return value
The player's position known by police. Vector zero if the player has no wanted level.
