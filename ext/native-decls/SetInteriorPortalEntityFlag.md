---
ns: CFX
apiset: client
game: gta5
---
## SET_INTERIOR_PORTAL_ENTITY_FLAG

```c
void SET_INTERIOR_PORTAL_ENTITY_FLAG(int interiorId, int portalIndex, int entityIndex, int flag);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)
local portalIndex = 0

if interiorId ~= 0 then
  local count = GetInteriorPortalEntityCount(interiorId, portalIndex)
  for i=0, count-1 do
    SetInteriorPortalEntityFlag(interiorId, portalIndex, i, 0)
  end
  RefreshInterior(interiorId)
end
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.
* **entityIndex**: Portal entity index.
* **flag**: New flag value.
