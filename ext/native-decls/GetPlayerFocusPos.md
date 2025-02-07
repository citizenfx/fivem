---
ns: CFX
apiset: server
---

## GET_PLAYER_FOCUS_POS

```c
Vector3 GET_PLAYER_FOCUS_POS(char* playerSrc);
```

Gets the focus position (i.e. the position of the active camera in the game world) of a player.

## Parameters
* **playerSrc**: The player to get the focus position of

## Return value
Returns a `Vector3` containing the focus position of the player.
