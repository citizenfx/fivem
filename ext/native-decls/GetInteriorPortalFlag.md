---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_PORTAL_FLAG

```c
int GET_INTERIOR_PORTAL_FLAG(int interiorId, int portalIndex);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local portalFlag = GetInteriorPortalFlag(interiorId, 0)
  print("portal 0 flag is: " .. portalRoomFrom)
end
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.

## Return value
Portal's flag.