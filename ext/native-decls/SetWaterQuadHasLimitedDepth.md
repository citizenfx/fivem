---
ns: CFX
apiset: client
game: gta5
---
## SET_WATER_QUAD_HAS_LIMITED_DEPTH

```c
BOOL SET_WATER_QUAD_HAS_LIMITED_DEPTH(int waterQuad, BOOL hasLimitedDepth);
```

## Parameters
* **waterQuad**: The water quad index
* **hasLimitedDepth**: Unknown effect

## Return value
Returns `true` if `waveQuad`s `hasLimitedDepth` flag got set, `false` otherwise.

## Examples

```lua
local success = SetWaterQuadHasLimitedDepth(0, true)
```
