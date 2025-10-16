---
ns: CFX
apiset: client
game: rdr3
---
## GET_INTERIOR_ROOM_REFLECTION_PROBE_EXTENTS

```c
void GET_INTERIOR_ROOM_REFLECTION_PROBE_EXTENTS(int interiorId, int roomId, int probeId, float* minX, float* minY, float* minZ, float* maxX, float* maxY, float* maxZ);
```

## Description

Gets the extents (bounding box) of a reflection probe in the specified room.

## Parameters

* **interiorId**: The interior ID
* **roomId**: The room ID
* **probeId**: The reflection probe ID
* **minX**: Pointer to store the minimum X coordinate
* **minY**: Pointer to store the minimum Y coordinate
* **minZ**: Pointer to store the minimum Z coordinate
* **maxX**: Pointer to store the maximum X coordinate
* **maxY**: Pointer to store the maximum Y coordinate
* **maxZ**: Pointer to store the maximum Z coordinate

## Examples

```lua
local interiorId = GetInteriorFromEntity(PlayerPedId())
local roomId = 0
local probeId = 0
local minX, minY, minZ = 0.0, 0.0, 0.0
local maxX, maxY, maxZ = 0.0, 0.0, 0.0

GET_INTERIOR_ROOM_REFLECTION_PROBE_EXTENTS(interiorId, roomId, probeId, minX, minY, minZ, maxX, maxY, maxZ)
print("Probe extents: min(" .. minX .. ", " .. minY .. ", " .. minZ .. ") max(" .. maxX .. ", " .. maxY .. ", " .. maxZ .. ")")
```

## Notes

- This native is only available in RDR3
- The pointers are not modified if the interior, room, or probe is invalid
