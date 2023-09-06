---
ns: CFX
apiset: client
game: gta5
---
## GET_WAVE_QUAD_DIRECTION

```c
BOOL GET_WAVE_QUAD_DIRECTION(int waveQuad, float* directionX, float* directionY);
```

## Examples

```lua
local success, directionX, directionY = GetWaveQuadDirection(1)
```

## Parameters
* **waveQuad**: The wave quad index
* **directionX**: The wave quad X direction
* **directionY**: The wave quad Y direction

## Return value
Returns true on success. Direction values are undefined on failure