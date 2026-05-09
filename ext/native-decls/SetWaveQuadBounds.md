---
ns: CFX
apiset: client
game: gta5
---
## SET_WAVE_QUAD_BOUNDS

```c
BOOL SET_WAVE_QUAD_BOUNDS(int waveQuad, int minX, int minY, int maxX, int maxY);
```

This native allows you to update the bounds of a specified water quad index.

## Parameters
* **waveQuad**: The wave quad index
* **minX**: The minX coordinate
* **minY**: The minY coordinate
* **maxX**: The maxX coordinate
* **maxY**: The maxY coordinate

## Return value
Returns `true` if `waveQuad`s bounds got set, `false` otherwise.

## Examples
```lua
local success = SetWaveQuadBounds(0, -5000, -5000, 5000, 5000)
```
