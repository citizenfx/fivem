---
ns: CFX
apiset: client
game: rdr3
---

## GET_INTERIOR_ROOM_REFLECTION_PROBE_GUID

```c
unsigned long long GET_INTERIOR_ROOM_REFLECTION_PROBE_GUID(int interiorId, int roomId, int probeId);
```

### Description
Gets the GUID of a reflection probe in an interior room.

### Parameters
- **interiorId** (int): The interior ID
- **roomId** (int): The room index
- **probeId** (int): The reflection probe index

### Returns
- **unsigned long long**: The GUID of the reflection probe, or 0 if invalid

### Example
```lua
local guid = GetInteriorRoomReflectionProbeGuid(interiorId, roomId, probeId)
if guid ~= 0 then
    print("Probe GUID: 0x" .. string.format("%016X", guid))
end
```

### Notes
- Returns 0 if the interior, room, or probe index is invalid
- GUID is a unique 64-bit identifier for the reflection probe
- RDR3 only
