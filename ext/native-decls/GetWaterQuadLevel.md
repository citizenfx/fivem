---
ns: CFX
apiset: client
game: gta5
---
## GET_WATER_QUAD_LEVEL

```c
BOOL GET_WATER_QUAD_LEVEL(int waterQuad, float* waterQuadLevel);
```

*level is defined as "z" in water.xml*

## Parameters
* **waterQuad**: The water quad index
* **waterQuadLevel**: The returned water quad level

## Return value
Returns `true` on success, `false` otherwise, `waterQuadLevel` will be set to `0.0` on failure.

## Examples
```lua
local success, waterQuadLevel = GetWaterQuadLevel(0)
```
