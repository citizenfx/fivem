---
ns: CFX
apiset: server
---
## GET_PLAYER_WANTED_CENTRE_POSITION

```c
Vector3 GET_PLAYER_WANTED_CENTRE_POSITION(char* playerSrc);
```

Gets the current known coordinates for the specified player from cops perspective.

## Parameters
* **playerSrc**: The target player

## Return value
Returns the player's position known by police, or `vector3(0, 0, 0)` if the player has no wanted level.
