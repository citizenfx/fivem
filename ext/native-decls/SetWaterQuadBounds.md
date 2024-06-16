---
ns: CFX
apiset: client
game: gta5
---
## SET_WATER_QUAD_BOUNDS

```c
BOOL SET_WATER_QUAD_BOUNDS(int waterQuad, int minX, int minY, int maxX, int maxY);
```

This native allows you to update the bounds of a specified water quad index.

## Examples

```lua
local success = SetWaterQuadBounds(0, -5000.0, -5000.0, 5000.0, 5000.0)
```

## Parameters
* **waterQuad**: The water quad index
* **minX**: The minX coordinate
* **minY**: The minY coordinate
* **maxX**: The maxX coordinate
* **maxY**: The maxY coordinate

## Return value
Returns true on success.