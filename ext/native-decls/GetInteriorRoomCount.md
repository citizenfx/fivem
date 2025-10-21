---
ns: CFX
apiset: client
---
## GET_INTERIOR_ROOM_COUNT

```c
int GET_INTERIOR_ROOM_COUNT(int interiorId);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local count = GetInteriorRoomCount(interiorId)
  print("interior " .. interiorId .. "has " .. count .. " rooms")
end
```

## Parameters
* **interiorId**: The target interior.

## Return value
The amount of rooms in interior.