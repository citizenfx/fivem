---
ns: CFX
apiset: client
game: gta5
---
## SET_CALMING_QUAD_BOUNDS

```c
BOOL SET_CALMING_QUAD_BOUNDS(int waterQuad, int minX, int minY, int maxX, int maxY);
```

## Examples

```lua
local success = SetCalmingQuadBounds(1, -500, -500, 500, 500)
```

## Parameters
* **waterQuad**: The calming quad index
* **minX**: The minX coordinate
* **minY**: The minY coordinate
* **maxX**: The maxX coordinate
* **maxY**: The maxY coordinate

## Return value
Returns true on success.