---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_ROOM_NAME

```c
char* GET_INTERIOR_ROOM_NAME(int interiorId, int roomIndex);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)
local roomHash = GetRoomKeyFromEntity(playerPed)
local roomId = GetInteriorRoomIndexByHash(interiorId, roomHash)

if roomId ~= -1 then
  local roomName = GetInteriorRoomName(interiorId, roomId)
  print("current room name is: " .. roomName)
end
```

## Parameters
* **interiorId**: The target interior.
* **roomIndex**: Interior room index.

## Return value
Room's name.
