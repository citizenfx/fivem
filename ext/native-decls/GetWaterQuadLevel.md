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

## Examples

```lua
local success, waterQuadLevel = GetWaterQuadLevel(0)
```

## Parameters
* **waterQuad**: The water quad index
* **waterQuad**: The returned water quad level
  
## Return value
Returns true on success. Level is undefined on failure