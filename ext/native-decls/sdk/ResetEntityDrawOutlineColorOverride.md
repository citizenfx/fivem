---
ns: CFX
apiset: client
game: gta5
---
## RESET_ENTITY_DRAW_OUTLINE_COLOR_OVERRIDE

```c
void RESET_ENTITY_DRAW_OUTLINE_COLOR_OVERRIDE(Entity entity);
```

Removes the outline color override set by [`SET_ENTITY_DRAW_OUTLINE_COLOR_OVERRIDE`](#_0xD09A4A30), so the entity is outlined with the global color from [`SET_ENTITY_DRAW_OUTLINE_COLOR`](#_0xB41A56C2) again.

## Parameters
* **entity**: A valid entity handle.
