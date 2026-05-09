---
ns: CFX
apiset: client
game: gta5
---
## GET_WATER_QUAD_NO_STENCIL

```c
BOOL GET_WATER_QUAD_NO_STENCIL(int waterQuad, int* noStencil);
```

## Parameters
* **waterQuad**: The water quad index
* **noStencil**: Returns `1` if the given water quad has no stencil

## Return value
Returns `true` on success, `false` otherwise, `noStencil` will be `0` on failure

## Examples
```lua
local success, noStencil = GetWaterQuadNoStencil(0)
```
