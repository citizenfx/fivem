---
ns: CFX
apiset: client
game: gta5
---
## SET_INTERIOR_PORTAL_ROOM_FROM

```c
void SET_INTERIOR_PORTAL_ROOM_FROM(int interiorId, int portalIndex, int roomFrom);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local portalIndex = 0

  SetInteriorPortalRoomFrom(interiorId, portalIndex, 0)
  RefreshInterior(interiorId)
end
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.
* **roomFrom**: New value.
