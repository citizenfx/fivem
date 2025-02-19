---
ns: CFX
apiset: client
game: gta5
---
## SET_COLSHAPE_SYSTEM_ENABLED

```c
void SET_COLSHAPE_SYSTEM_ENABLED(bool activated);
```

Activates the grid-based collision detection system. If enabled, the game will fire `onPlayerEnterColshape` and `onPlayerLeaveColshape` if you've created collision shapes and `PlayerPedId()` collides with those shapes.

## Parameters
* **activated**: Whether the System should be active (true) or not (false)