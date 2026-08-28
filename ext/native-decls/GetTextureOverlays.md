---
ns: CFX
apiset: client
game: rdr3
---
## GET_TEXTURE_OVERLAYS

```c
int GET_TEXTURE_OVERLAYS(int textureId);
```

Returns the number of overlays currently used by the texture (max `16`). To check how many more overlays can be added to this texture use `16 - GET_TEXTURE_OVERLAYS(textureId)`.

## Parameters
* **textureId**: texture id created by `0xC5E7204F322E49EB` (or returned by `GET_PLAYER_TEXTURE`).

## Returns
Overlay count of the texture (`0`-`16`), or `0` if the texture does not exist.
