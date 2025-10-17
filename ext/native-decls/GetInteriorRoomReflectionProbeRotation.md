---
ns: CFX
apiset: client
game: rdr3
---
## GET_INTERIOR_ROOM_REFLECTION_PROBE_ROTATION

```c
void GET_INTERIOR_ROOM_REFLECTION_PROBE_ROTATION(int interiorId, int roomId, int probeId, float* x, float* y, float* z, float* w);
```

## Description

Gets the rotation quaternion of a reflection probe in the specified room.

## Parameters

* **interiorId**: The interior ID
* **roomId**: The room ID
* **probeId**: The reflection probe ID
* **x**: Pointer to store the X component of the quaternion
* **y**: Pointer to store the Y component of the quaternion
* **z**: Pointer to store the Z component of the quaternion
* **w**: Pointer to store the W component of the quaternion

## Examples

```lua
local interiorId = GetInteriorFromEntity(PlayerPedId())
local roomId = 0
local probeId = 0
local x, y, z, w = 0.0, 0.0, 0.0, 0.0

GET_INTERIOR_ROOM_REFLECTION_PROBE_ROTATION(interiorId, roomId, probeId, x, y, z, w)
print("Probe rotation: (" .. x .. ", " .. y .. ", " .. z .. ", " .. w .. ")")
```

## Notes

- This native is only available in RDR3
- The pointers are not modified if the interior, room, or probe is invalid
- The rotation is stored as a quaternion (x, y, z, w)
