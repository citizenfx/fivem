---
ns: CFX
apiset: client
game: rdr3
---
## SET_INTERIOR_ROOM_REFLECTION_PROBE_INFLUENCE_EXTENTS

```c
void SET_INTERIOR_ROOM_REFLECTION_PROBE_INFLUENCE_EXTENTS(int interiorId, int roomId, int probeId, float x, float y, float z);
```

## Description

Sets the influence extents of a reflection probe in the specified room.

## Parameters

* **interiorId**: The interior ID
* **roomId**: The room ID
* **probeId**: The reflection probe ID
* **x**: The X influence extent
* **y**: The Y influence extent
* **z**: The Z influence extent

## Examples

```lua
local interiorId = GetInteriorFromEntity(PlayerPedId())
local roomId = 0
local probeId = 0

-- Set probe influence extents
SET_INTERIOR_ROOM_REFLECTION_PROBE_INFLUENCE_EXTENTS(interiorId, roomId, probeId, 1.0, 1.0, 1.0)
```

## Notes

- This native is only available in RDR3
- Changes are applied immediately to the reflection probe
- No effect if the interior, room, or probe is invalid
