---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_PORTAL_ENTITY_COUNT

```c
int GET_INTERIOR_PORTAL_ENTITY_COUNT(int interiorId, int portalIndex);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)
local portalIndex = 0

if interiorId ~= 0 then
  local count = GetInteriorPortalEntityCount(interiorId, portalIndex)
  print("portal " .. portalIndex .." entity count is: " .. count)
end
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.

## Return value
Portal entity count.