---
ns: CFX
apiset: client
game: gta5
---
## GET_WAVE_QUAD_AT_COORDS

```c
int GET_WAVE_QUAD_AT_COORDS(float x, float y);
```

This native returns the index of a wave quad if the given point is inside its bounds.

## Examples

```lua
local currentPedPosition = GetEntityCoords(PlayerPedId())
local waveQuadIndex = GetWaveQuadAtCoords(currentPedPosition.x, currentPedPosition.y)
```

## Parameters
* **x**: The X coordinate
* **y**: The Y coordinate

## Return value
The wave quad index at the given position. Returns -1 if there isn't any there.