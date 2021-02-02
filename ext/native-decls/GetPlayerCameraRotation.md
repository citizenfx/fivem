---
ns: CFX
apiset: server
---

## GET_PLAYER_CAMERA_ROTATION

```c
Vector3 GET_PLAYER_CAMERA_ROTATION(char* playerSrc);
```

Gets the current camera rotation for a specified player. This native is used server side when using OneSync.

## Parameters

- **playerSrc**: The player handle.

## Return value

The player's camera rotation. Values are in radians.