---
ns: CFX
apiset: client
game: gta5
---
## SET_INTERIOR_ROOM_FLAG

```c
void SET_INTERIOR_ROOM_FLAG(int interiorId, int roomIndex, int flag);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)
local roomHash = GetRoomKeyFromEntity(playerPed)
local roomId = GetInteriorRoomIndexByHash(interiorId, roomHash)

if roomId ~= -1 then
  SetInteriorRoomFlag(interiorId, roomId, 64)
  RefreshInterior(interiorId)
end
```

## Parameters
* **interiorId**: The target interior.
* **roomIndex**: Interior room index.
* **flag**: New flag value.
