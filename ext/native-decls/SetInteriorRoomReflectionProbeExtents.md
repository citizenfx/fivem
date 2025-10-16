---
ns: CFX
apiset: client
game: rdr3
---
## SET_INTERIOR_ROOM_REFLECTION_PROBE_EXTENTS

```c
void SET_INTERIOR_ROOM_REFLECTION_PROBE_EXTENTS(int interiorId, int roomId, int probeId, float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
```

## Description

Sets the extents (bounding box) of a reflection probe in the specified room.

## Parameters

* **interiorId**: The interior ID
* **roomId**: The room ID
* **probeId**: The reflection probe ID
* **minX**: The minimum X coordinate
* **minY**: The minimum Y coordinate
* **minZ**: The minimum Z coordinate
* **maxX**: The maximum X coordinate
* **maxY**: The maximum Y coordinate
* **maxZ**: The maximum Z coordinate

## Examples

```lua
local interiorId = GetInteriorFromEntity(PlayerPedId())
local roomId = 0
local probeId = 0

-- Set probe extents to a 10x10x5 box centered at origin
SET_INTERIOR_ROOM_REFLECTION_PROBE_EXTENTS(interiorId, roomId, probeId, -5.0, -5.0, -2.5, 5.0, 5.0, 2.5)
```

## Notes

- This native is only available in RDR3
- Changes are applied immediately to the reflection probe
- No effect if the interior, room, or probe is invalid
