---
ns: CFX
apiset: client
game: rdr3
---
## GET_INTERIOR_ROOM_REFLECTION_PROBE_COUNT

```c
int GET_INTERIOR_ROOM_REFLECTION_PROBE_COUNT(int interiorId, int roomId);
```

## Description

Returns the number of reflection probes in the specified room.

## Parameters

* **interiorId**: The interior ID
* **roomId**: The room ID

## Return value

The number of reflection probes in the room.

## Examples

```lua
local interiorId = GetInteriorFromEntity(PlayerPedId())
local roomId = 0
local probeCount = GET_INTERIOR_ROOM_REFLECTION_PROBE_COUNT(interiorId, roomId)
print("Room has " .. probeCount .. " reflection probes")
```

## Notes

- This native is only available in RDR3
- Returns 0 if the interior or room is invalid
