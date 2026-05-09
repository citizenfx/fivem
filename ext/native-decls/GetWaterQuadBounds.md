---
ns: CFX
apiset: client
game: gta5
---
## GET_WATER_QUAD_BOUNDS

```c
BOOL GET_WATER_QUAD_BOUNDS(int waterQuad, int* minX, int* minY, int* maxX, int* maxY);
```

## Parameters
* **waterQuad**: The water quad index
* **minX**: The minX coordinate
* **minY**: The minY coordinate
* **maxX**: The maxX coordinate
* **maxY**: The maxY coordinate

## Return value
Returns `true` if the operation was successful, `false` otherwise, the bounds will be set to `0` on failure.

## Examples
```lua
local success, minX, minY, maxX, maxY = GetWaterQuadBounds(1)
```
