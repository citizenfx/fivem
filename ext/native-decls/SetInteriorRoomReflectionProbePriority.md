---
ns: CFX
apiset: client
game: rdr3
---
## SET_INTERIOR_ROOM_REFLECTION_PROBE_PRIORITY

```c
void SET_INTERIOR_ROOM_REFLECTION_PROBE_PRIORITY(int interiorId, int roomId, int probeId, int priority);
```

## Description

Sets the priority of a reflection probe in the specified room.

## Parameters

* **interiorId**: The interior ID
* **roomId**: The room ID
* **probeId**: The reflection probe ID
* **priority**: The priority value (0-255)

## Examples

```lua
local interiorId = GetInteriorFromEntity(PlayerPedId())
local roomId = 0
local probeId = 0

-- Set probe priority to maximum
SET_INTERIOR_ROOM_REFLECTION_PROBE_PRIORITY(interiorId, roomId, probeId, 255)
```

## Notes

- This native is only available in RDR3
- Changes are applied immediately to the reflection probe
- No effect if the interior, room, or probe is invalid
- Priority values should be between 0 and 255
