---
ns: CFX
apiset: client
game: rdr3
---
## GET_INTERIOR_ROOM_REFLECTION_PROBE_PRIORITY

```c
int GET_INTERIOR_ROOM_REFLECTION_PROBE_PRIORITY(int interiorId, int roomId, int probeId);
```

## Description

Gets the priority of a reflection probe in the specified room.

## Parameters

* **interiorId**: The interior ID
* **roomId**: The room ID
* **probeId**: The reflection probe ID

## Return value

The priority of the reflection probe (0-255).

## Examples

```lua
local interiorId = GetInteriorFromEntity(PlayerPedId())
local roomId = 0
local probeId = 0
local priority = GET_INTERIOR_ROOM_REFLECTION_PROBE_PRIORITY(interiorId, roomId, probeId)
print("Probe priority: " .. priority)
```

## Notes

- This native is only available in RDR3
- Returns 0 if the interior, room, or probe is invalid
- Priority values range from 0 to 255
