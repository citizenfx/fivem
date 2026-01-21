---
ns: CFX
apiset: client
game: gta5
---
## GET_WATER_QUAD_AT_COORDS

```c
int GET_WATER_QUAD_AT_COORDS(float x, float y);
```

If you also want to check for water level, check out [`GetWaterQuadAtCoords_3d`](#_0xF8E03DB8)

## Parameters
* **x**: The X coordinate
* **y**: The Y coordinate

## Return value
Returns the water quad index at the given position, or `-1` if there isn't one.

## Examples

```lua
local currentPedPosition = GetEntityCoords(PlayerPedId())
local waterQuadIndex = GetWaterQuadAtCoords(currentPedPosition.x, currentPedPosition.y)
```
