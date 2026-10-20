---
ns: CFX
apiset: server
---
## SET_PLAYER_NAME

```c
BOOL SET_PLAYER_NAME(char* playerSrc, char* name);
```

Changes the name of a player, both for the server (`GET_PLAYER_NAME`, player lists) and for other
clients in-game.

The name gets normalized the same way names sent on connection are: it is limited to 200 bytes and
invalid UTF-8 sequences get replaced.

This can be used during `playerConnecting` as well as afterwards. Changing the name triggers the
`onPlayerNameChanged` server event.

## Parameters
* **playerSrc**: The player to change the name of.
* **name**: The new name for the player.

## Return value
`true` if the name was changed, `false` if the player didn't exist, the name was empty/invalid, or
the player already had this name.
