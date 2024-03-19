---
ns: CFX
apiset: client
game: gta5
---
## SET_INTERIOR_PORTAL_FLAG

```c
void SET_INTERIOR_PORTAL_FLAG(int interiorId, int portalIndex, int flag);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local portalIndex = 0

  SetInteriorPortalFlag(interiorId, portalIndex, 1)
  RefreshInterior(interiorId)
end
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.
* **flag**: New flag value.
