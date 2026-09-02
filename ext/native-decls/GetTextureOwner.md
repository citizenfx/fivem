---
ns: CFX
apiset: client
game: rdr3
---
## GET_TEXTURE_OWNER

```c
int GET_TEXTURE_OWNER(int textureId);
```

Returns the player ID that owns the given texture override.

## Parameters
* **textureId**: texture id created by `0xC5E7204F322E49EB` (or returned by `GET_PLAYER_TEXTURE`).

## Returns
Player handle or `-1` if the texture does not exist.
