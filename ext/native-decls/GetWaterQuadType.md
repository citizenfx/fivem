---
ns: CFX
apiset: client
game: gta5
---
## GET_WATER_QUAD_TYPE

```c
BOOL GET_WATER_QUAD_TYPE(int waterQuad, int* waterType);
```

Valid type definitions:

* **0** Square
* **1** Right triangle where the 90 degree angle is at maxX, minY
* **2** Right triangle where the 90 degree angle is at minX, minY
* **3** Right triangle where the 90 degree angle is at minX, maxY
* **4** Right triangle where the 90 degree angle is at maxY, maxY

## Parameters
* **waterQuad**: The water quad index
* **waterType**: The type of water quad it is

## Return value
Returns `true` on success, `false` otherwise, `waterType` will be `-1` on failure

## Examples
```lua
local success, type = GetWaterQuadType(0)
```
