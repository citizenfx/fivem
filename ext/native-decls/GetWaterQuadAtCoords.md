---
ns: CFX
apiset: client
game: gta5
---
## GET_WATER_QUAD_AT_COORDS

```c
int GET_WATER_QUAD_AT_COORDS(float x, float y);
```

This native returns the index of a water quad if the given point is inside its bounds.

*If you also want to check for water level, check out [`GetWaterQuadAtCoords_3d`](#_0xF8E03DB8)*

## Examples

```lua
local currentPedPosition = GetEntityCoords(PlayerPedId())
local waterQuadIndex = GetWaterQuadAtCoords(currentPedPosition.x, currentPedPosition.y)
```

## Parameters
* **x**: The X coordinate
* **y**: The Y coordinate

## Return value
The water quad index at the given position. Returns -1 if there isn't any there.