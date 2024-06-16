---
ns: CFX
apiset: client
game: gta5
---
## SET_WATER_QUAD_TYPE

```c
BOOL SET_WATER_QUAD_TYPE(int waterQuad, int type);
```

This native allows you to update the water quad type.

Valid type definitions:

* **0** Square
* **1** Right triangle where the 90 degree angle is at maxX, minY
* **2** Right triangle where the 90 degree angle is at minX, minY
* **3** Right triangle where the 90 degree angle is at minX, maxY
* **4** Right triangle where the 90 degree angle is at maxY, maxY

## Examples

```lua
local success = SetWaterQuadType(0, 0)
```

## Parameters
* **waterQuad**: The water quad index
* **type**: The water quad type

## Return value
Returns true on success.