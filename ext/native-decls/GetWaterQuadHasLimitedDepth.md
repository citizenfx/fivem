---
ns: CFX
apiset: client
game: gta5
---
## GET_WATER_QUAD_HAS_LIMITED_DEPTH

```c
BOOL GET_WATER_QUAD_HAS_LIMITED_DEPTH(int waterQuad, int* hasLimitedDepth);
```
## Examples

```lua
local success, hasLimitedDepth = GetWaterQuadHasLimitedDepth(0)
```

## Parameters
* **waterQuad**: The water quad index

## Return value
Returns if the given water quad has a limited depth.