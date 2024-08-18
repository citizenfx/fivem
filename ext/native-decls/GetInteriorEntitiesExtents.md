---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_ENTITIES_EXTENTS

```c
void GET_INTERIOR_ENTITIES_EXTENTS(int interiorId, float* bbMinX, float* bbMinY, float* bbMinZ, float* bbMaxX, float* bbMaxY, float* bbMaxZ);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local minX, minY, minZ, maxX, maxY, maxZ = GetInteriorEntitiesExtents(interiorId, roomId)
  print("current entities extents is: " .. vec(minX, minY, minZ) .." / " .. vec(maxX, maxY, maxZ))
end
```

## Parameters
* **interiorId**: The target interior.

## Return value
Interior entities extents.
