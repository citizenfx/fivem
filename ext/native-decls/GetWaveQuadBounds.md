---
ns: CFX
apiset: client
game: gta5
---
## GET_WAVE_QUAD_BOUNDS

```c
BOOL GET_WAVE_QUAD_BOUNDS(int waveQuad, int* minX, int* minY, int* maxX, int* maxY);
```

## Examples

```lua
local success, minX, minY, maxX, maxY = GetWaveQuadBounds(1)
```

## Parameters
* **waveQuad**: The wave quad index
* **minX**: The minX coordinate
* **minY**: The minY coordinate
* **maxX**: The maxX coordinate
* **maxY**: The maxY coordinate

## Return value
Returns true on success. Bounds are undefined on failure