---
ns: CFX
apiset: server
---
## SET_PLAYER_CULLING_RADIUS

```c
void SET_PLAYER_CULLING_RADIUS(char* playerSrc, float radius);
```

Sets the culling radius for the specified player.
Set to `0.0` to reset.

**WARNING**: Culling natives are deprecated and have known, [unfixable issues](https://forum.cfx.re/t/issue-with-culling-radius-and-server-side-entities/4900677/4)

## Parameters
* **playerSrc**: The player to set the culling radius for.
* **radius**: The radius.
