---
ns: CFX
apiset: client
game: rdr3
---
## SET_INTERIOR_ROOM_REFLECTION_PROBE_CENTER_OFFSET

```c
void SET_INTERIOR_ROOM_REFLECTION_PROBE_CENTER_OFFSET(int interiorId, int roomId, int probeId, float x, float y, float z);
```

## Description

Sets the center offset of a reflection probe in the specified room.

## Parameters

* **interiorId**: The interior ID
* **roomId**: The room ID
* **probeId**: The reflection probe ID
* **x**: The X coordinate offset
* **y**: The Y coordinate offset
* **z**: The Z coordinate offset

## Examples

```lua
local interiorId = GetInteriorFromEntity(PlayerPedId())
local roomId = 0
local probeId = 0

-- Set probe center offset
SET_INTERIOR_ROOM_REFLECTION_PROBE_CENTER_OFFSET(interiorId, roomId, probeId, 0.0, 2.0, -0.8)
```

## Notes

- This native is only available in RDR3
- Changes are applied immediately to the reflection probe
- No effect if the interior, room, or probe is invalid
