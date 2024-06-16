---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_PORTAL_ROOM_TO

```c
int GET_INTERIOR_PORTAL_ROOM_TO(int interiorId, int portalIndex);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local roomIndex = 0

  local portalRoomTo = GetInteriorPortalRoomTo(interiorId, 0)
  print("portal " .. roomIndex .. " room TO is: " .. portalRoomTo)
end
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.

## Return value
Portal's room TO index.