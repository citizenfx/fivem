---
ns: CFX
apiset: client
game: gta5
---
## SET_WATER_QUAD_IS_INVISIBLE

```c
BOOL SET_WATER_QUAD_IS_INVISIBLE(int waterQuad, BOOL isInvisible);
```

## Parameters
* **waterQuad**: The water quad index
* **isInvisible**: Unknown effect

## Return value
Returns `true` if `waveQuad`s `isInvisible` flag got set, `false` otherwise.

## Examples

```lua
local success = SetWaterQuadIsInvisible(0, true)
```
