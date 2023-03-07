---
ns: CFX
apiset: client
game: gta5
---
## GET_CALMING_QUAD_AT_COORDS

```c
int GET_CALMING_QUAD_AT_COORDS(float x, float y);
```

This native returns the index of a calming quad if the given point is inside its bounds.

## Examples

```lua
local currentPedPosition = GetEntityCoords(PlayerPedId())
local calmingQuadIndex = GetCalmingQuadAtCoords(currentPedPosition.x, currentPedPosition.y)
```

## Parameters
* **x**: The X coordinate
* **y**: The Y coordinate

## Return value
The calming quad index at the given position. Returns -1 if there isn't any there.