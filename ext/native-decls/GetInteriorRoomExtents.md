---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_ROOM_EXTENTS

```c
void GET_INTERIOR_ROOM_EXTENTS(int interiorId, int roomIndex, float* bbMinX, float* bbMinY, float* bbMinZ, float* bbMaxX, float* bbMaxY, float* bbMaxZ);
```


## Parameters
* **interiorId**: The target interior.
* **roomIndex**: Interior room index.
* **bbMinX**: The minimum X bounding box.
* **bbMinY**: The minimum Y bounding box.
* **bbMinZ**: The minimum Z bounding box.
* **bbMaxX**: The maximum X bounding box.
* **bbMaxY**: The maximum Y bounding box.
* **bbMaxZ**: The maximum Z bounding box.

## Return value
Returns the room extents.

## Examples
```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)
local roomHash = GetRoomKeyFromEntity(playerPed)
local roomId = GetInteriorRoomIndexByHash(interiorId, roomHash)

if roomId ~= -1 then
  local minX, minY, minZ, maxX, maxY, maxZ = GetInteriorRoomExtents(interiorId, roomId)
  print("current room extents is: " .. vec(minX, minY, minZ) .." / " .. vec(maxX, maxY, maxZ))
end
```
