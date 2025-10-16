---
ns: CFX
apiset: client
game: rdr3
---
## GET_INTERIOR_ROOM_REFLECTION_PROBE_CENTER_OFFSET

```c
void GET_INTERIOR_ROOM_REFLECTION_PROBE_CENTER_OFFSET(int interiorId, int roomId, int probeId, float* x, float* y, float* z);
```

## Description

Gets the center offset of a reflection probe in the specified room.

## Parameters

* **interiorId**: The interior ID
* **roomId**: The room ID
* **probeId**: The reflection probe ID
* **x**: Pointer to store the X coordinate
* **y**: Pointer to store the Y coordinate
* **z**: Pointer to store the Z coordinate

## Examples

```lua
local interiorId = GetInteriorFromEntity(PlayerPedId())
local roomId = 0
local probeId = 0
local x, y, z = 0.0, 0.0, 0.0

GET_INTERIOR_ROOM_REFLECTION_PROBE_CENTER_OFFSET(interiorId, roomId, probeId, x, y, z)
print("Probe center offset: (" .. x .. ", " .. y .. ", " .. z .. ")")
```

## Notes

- This native is only available in RDR3
- The pointers are not modified if the interior, room, or probe is invalid
