---
ns: CFX
apiset: client
game: rdr3
---
## GET_TEXTURE_COMPONENT_CATEGORY

```c
int GET_TEXTURE_COMPONENT_CATEGORY(int textureId);
```

Returns the category hash of the component that the given texture override belongs to.

## Parameters
* **textureId**: texture id created by `0xC5E7204F322E49EB` (or returned by `GET_PLAYER_TEXTURE`).

## Returns
Category hash identifying the component (e.g. HEADS, BODIES_UPPER, BODIES_LOWER), or `0` if the texture does not exist.