---
ns: CFX
apiset: client
game: gta5
---
## SET_ENTITY_DRAW_OUTLINE_SHADER

```c
void SET_ENTITY_DRAW_OUTLINE_SHADER(int shader);
```

Sets variant of shader that will be used to draw entity outline.

Variants are:
* **0**: Default value, gauss shader.
* **1**: 2px wide solid color outline.
* **2**: Fullscreen solid color except for entity.

## Parameters
* **shader**: An outline shader variant.
