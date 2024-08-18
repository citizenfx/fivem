---
ns: CFX
apiset: client
game: gta5
---
## GET_WATER_QUAD_NO_STENCIL

```c
BOOL GET_WATER_QUAD_NO_STENCIL(int waterQuad, int* noStencil);
```
## Examples

```lua
local success, noStencil = GetWaterQuadNoStencil(0)
```

## Parameters
* **waterQuad**: The water quad index

## Return value
Returns if the given water quad has no stencil.