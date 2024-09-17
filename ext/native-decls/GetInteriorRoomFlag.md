---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_ROOM_FLAG

```c
int GET_INTERIOR_ROOM_FLAG(int interiorId, int roomIndex);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)
local roomHash = GetRoomKeyFromEntity(playerPed)
local roomId = GetInteriorRoomIndexByHash(interiorId, roomHash)

if roomId ~= -1 then
  local roomFlag = GetInteriorRoomFlag(interiorId, roomId)
  print("current room flag is: " .. roomFlag)
end
```

## Parameters
* **interiorId**: The target interior.
* **roomIndex**: Interior room index.

## Return value
Room's flag.
