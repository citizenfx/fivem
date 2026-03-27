---
ns: CFX
apiset: client
game: gta5
---
## GET_WATER_QUAD_AT_COORDS_3D

```c
int GET_WATER_QUAD_AT_COORDS_3D(float x, float y, float z);
```

This alternative implementation of [`GetWaterQuadAtCoords`](#_0x17321452) also checks the height of the water level.

## Parameters
* **x**: The X coordinate
* **y**: The Y coordinate
* **z**: The water level inside the water quad

## Return value
Returns the water quad index at the given position, or `-1` if the point is above water level or doesn't exist.

## Examples

```lua
local currentPedPosition = GetEntityCoords(PlayerPedId())
local waterQuadIndex = GetWaterQuadAtCoords(currentPedPosition.x, currentPedPosition.y, currentPedPosition.z)
```
