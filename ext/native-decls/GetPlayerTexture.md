---
ns: CFX
apiset: client
game: rdr3
---
## GET_PLAYER_TEXTURE

```c
int GET_PLAYER_TEXTURE(int playerId, int categoryHash);
```

Returns a texture ID for the given player that match the specified category hash. Returns -1 if no active texture overrides exist for that combination.

## Parameters
* **playerId**: player ID (e.g. from `PLAYER_ID` / `NETWORK_PLAYER_ID_TO_INT`) — the same value the engine stores as the texture owner.
* **categoryHash**: category hash to filter textures by (e.g., HEADS, BODIES_UPPER, BODIES_LOWER).

## Returns
Texture ID (same format accepted by `DOES_TEXTURE_EXIST` / `REMOVE_TEXTURE`), or -1 if not found.
