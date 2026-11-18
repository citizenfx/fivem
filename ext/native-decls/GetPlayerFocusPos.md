---
ns: CFX
apiset: server
---

## GET_PLAYER_FOCUS_POS

```c
Vector3 GET_PLAYER_FOCUS_POS(char* playerSrc);
```

Gets the focus position (i.e. the position of the active camera in the game world) of a player.

**Note**: This is not always the actual camera position. When the game doesn't sync the camera position, this returns the player position. When the player is in a vehicle, the camera offset is added to the vehicle position instead of the ped position.

## Parameters
* **playerSrc**: The player to get the focus position of

## Return value
Returns a `Vector3` containing the focus position of the player.
