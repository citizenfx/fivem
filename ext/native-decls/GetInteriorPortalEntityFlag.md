---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_PORTAL_ENTITY_FLAG

```c
int GET_INTERIOR_PORTAL_ENTITY_FLAG(int interiorId, int portalIndex, int entityIndex);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)
local portalIndex = 0

if interiorId ~= 0 then
  local count = GetInteriorPortalEntityCount(interiorId, portalIndex)
  for i=0, count-1 do
    local flag = GetInteriorPortalEntityFlag(interiorId, portalIndex, i)
    print("portal " .. portalIndex .." entity " .. i .. " flag is: " .. flag)
  end
end
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.
* **entityIndex**: Portal entity index.

## Return value
Portal entity flag.