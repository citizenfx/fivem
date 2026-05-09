---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_ENTITIES_EXTENTS

```c
void GET_INTERIOR_ENTITIES_EXTENTS(int interiorId, float* bbMinX, float* bbMinY, float* bbMinZ, float* bbMaxX, float* bbMaxY, float* bbMaxZ);
```

## Parameters
* **interiorId**: The target interior.
* **bbMinX**: The minimum X bounding box.
* **bbMinY**: The minimum Y bounding box.
* **bbMinZ**: The minimum Z bounding box.
* **bbMaxX**: The maximum X bounding box.
* **bbMaxY**: The maximum Y bounding box.
* **bbMaxZ**: The maximum Z bounding box.

## Return value
Returns the minimum and maximum bounding box for the entities of `interiorId`

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local minX, minY, minZ, maxX, maxY, maxZ = GetInteriorEntitiesExtents(interiorId, roomId)
  print("current entities extents is: " .. vec(minX, minY, minZ) .." / " .. vec(maxX, maxY, maxZ))
end
```
