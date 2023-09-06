---
ns: CFX
apiset: client
game: gta5
---
## GET_CALMING_QUAD_BOUNDS

```c
BOOL GET_CALMING_QUAD_BOUNDS(int waterQuad, int* minX, int* minY, int* maxX, int* maxY);
```

## Examples

```lua
local success, minX, minY, maxX, maxY = GetCalmingQuadBounds(1)
```

## Parameters
* **waterQuad**: The calming quad index
* **minX**: The minX coordinate
* **minY**: The minY coordinate
* **maxX**: The maxX coordinate
* **maxY**: The maxY coordinate

## Return value
Returns true on success. Bounds are undefined on failure