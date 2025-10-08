---
ns: CFX
apiset: client
game: gta5
---
## SET_INTERIOR_PORTAL_ROOM_TO

```c
void SET_INTERIOR_PORTAL_ROOM_TO(int interiorId, int portalIndex, int roomTo);
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.
* **roomTo**: New value.

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local portalIndex = 0

  SetInteriorPortalRoomTo(interiorId, portalIndex, 0)
  RefreshInterior(interiorId)
end
```
