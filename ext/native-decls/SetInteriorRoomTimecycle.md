---
ns: CFX
apiset: client
game: gta5
---
## SET_INTERIOR_ROOM_TIMECYCLE

```c
void SET_INTERIOR_ROOM_TIMECYCLE(int interiorId, int roomIndex, int timecycleHash);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)
local roomHash = GetRoomKeyFromEntity(playerPed)
local roomId = GetInteriorRoomIndexByHash(interiorId, roomHash)

if roomId ~= -1 then
  local timecycleHash = GetHashKey("scanline_cam")
  SetInteriorRoomTimecycle(interiorId, roomId, timecycleHash)
  RefreshInterior(interiorId)
end
```

## Parameters
* **interiorId**: The target interior.
* **roomIndex**: Interior room index.
* **timecycleHash**: Timecycle hash.
