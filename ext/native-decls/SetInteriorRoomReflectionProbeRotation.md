---
ns: CFX
apiset: client
game: rdr3
---
## SET_INTERIOR_ROOM_REFLECTION_PROBE_ROTATION

```c
void SET_INTERIOR_ROOM_REFLECTION_PROBE_ROTATION(int interiorId, int roomId, int probeId, float x, float y, float z, float w);
```

## Description

Sets the rotation quaternion of a reflection probe in the specified room.

## Parameters

* **interiorId**: The interior ID
* **roomId**: The room ID
* **probeId**: The reflection probe ID
* **x**: The X component of the quaternion
* **y**: The Y component of the quaternion
* **z**: The Z component of the quaternion
* **w**: The W component of the quaternion

## Examples

```lua
local interiorId = GetInteriorFromEntity(PlayerPedId())
local roomId = 0
local probeId = 0

-- Set probe rotation (no rotation)
SET_INTERIOR_ROOM_REFLECTION_PROBE_ROTATION(interiorId, roomId, probeId, 0.0, 0.0, 0.0, 1.0)
```

## Notes

- This native is only available in RDR3
- Changes are applied immediately to the reflection probe
- No effect if the interior, room, or probe is invalid
- The rotation is stored as a quaternion (x, y, z, w)
