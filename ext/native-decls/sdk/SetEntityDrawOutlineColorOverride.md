---
ns: CFX
apiset: client
game: gta5
---
## SET_ENTITY_DRAW_OUTLINE_COLOR_OVERRIDE

```c
void SET_ENTITY_DRAW_OUTLINE_COLOR_OVERRIDE(Entity entity, int red, int green, int blue, int alpha);
```

Sets an outline color for a specific entity, overriding the global color set by [`SET_ENTITY_DRAW_OUTLINE_COLOR`](#_0xB41A56C2) for that entity only.

Outlined entities are grouped by color and each group is drawn in its own screen-space pass, so entities with different colors can be outlined at the same time. Entities without an override keep using the global color. The override is kept while the entity exists, even if its outline is toggled off and on again, and can be removed with [`RESET_ENTITY_DRAW_OUTLINE_COLOR_OVERRIDE`](#_0x71B72C6B).

The cost of outline rendering scales with the number of distinct colors visible at once rather than with the number of entities.

## Parameters
* **entity**: A valid entity handle.
* **red**: Red component of color.
* **green**: Green component of color.
* **blue**: Blue component of color.
* **alpha**: Alpha component of color, ignored for shader `0`.
