---
ns: CFX
apiset: client
game: gta5
---
## SET_INTERIOR_ROOM_EXTENTS

```c
void SET_INTERIOR_ROOM_EXTENTS(int interiorId, int roomIndex, float bbMinX, float bbMinY, float bbMinZ, float bbMaxX, float bbMaxY, float bbMaxZ);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  SetInteriorRoomExtents(interiorId, 0, -999.0, -999.0, -100.0, 999.0, 999.0, 100.0) -- 0 is a limbo usually
  RefreshInterior(interiorId)
end
```

## Parameters
* **interiorId**: The target interior.
* **roomIndex**: Interior room index.
* **bbMinX**:
* **bbMinY**:
* **bbMinZ**:
* **bbMaxX**:
* **bbMaxY**:
* **bbMaxZ**:
