---
ns: CFX
apiset: server
---
## GET_PLAYER_SCOPE

```c
object GET_PLAYER_SCOPE(char* playerSrc);
```

Returns all players in a player's scope including the owner.
The data returned adheres to the following layout:
```
[127, 42, 13, 37]
```

## Return value
An object containing a list of players.
