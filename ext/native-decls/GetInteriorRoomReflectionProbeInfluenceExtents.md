---
ns: CFX
apiset: client
game: rdr3
---
## GET_INTERIOR_ROOM_REFLECTION_PROBE_INFLUENCE_EXTENTS

```c
void GET_INTERIOR_ROOM_REFLECTION_PROBE_INFLUENCE_EXTENTS(int interiorId, int roomId, int probeId, float* x, float* y, float* z);
```

## Description

Gets the influence extents of a reflection probe in the specified room.

## Parameters

* **interiorId**: The interior ID
* **roomId**: The room ID
* **probeId**: The reflection probe ID
* **x**: Pointer to store the X influence extent
* **y**: Pointer to store the Y influence extent
* **z**: Pointer to store the Z influence extent

## Examples

```lua
local interiorId = GetInteriorFromEntity(PlayerPedId())
local roomId = 0
local probeId = 0
local x, y, z = 0.0, 0.0, 0.0

GET_INTERIOR_ROOM_REFLECTION_PROBE_INFLUENCE_EXTENTS(interiorId, roomId, probeId, x, y, z)
print("Probe influence extents: (" .. x .. ", " .. y .. ", " .. z .. ")")
```

## Notes

- This native is only available in RDR3
- The pointers are not modified if the interior, room, or probe is invalid
