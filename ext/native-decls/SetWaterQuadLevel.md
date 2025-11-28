---
ns: CFX
apiset: client
game: gta5
---
## SET_WATER_QUAD_LEVEL

```c
BOOL SET_WATER_QUAD_LEVEL(int waterQuad, float level);
```

## Parameters
* **waterQuad**: The water quad index
* **level**: The water level inside the water quad

## Return value
Returns `true` if `waveQuad`s level got set, `false` otherwise.

## Examples

```lua
local success = SetWaterQuadLevel(0, 55.0)
```
