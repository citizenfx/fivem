---
ns: CFX
apiset: client
game: gta5
---
## GET_WATER_QUAD_HAS_LIMITED_DEPTH

```c
BOOL GET_WATER_QUAD_HAS_LIMITED_DEPTH(int waterQuad, int* hasLimitedDepth);
```

## Parameters
* **waterQuad**: The water quad index
* **hasLimitedDepth**: Will be set to `1` if the quad has limited depth, `0` otherwise.

## Return value
Returns `true` if the operation was successful, `false` otherwise, `hasLimitedDepth` will be set to `0` on failure.

## Examples

```lua
local success, hasLimitedDepth = GetWaterQuadHasLimitedDepth(0)
```
