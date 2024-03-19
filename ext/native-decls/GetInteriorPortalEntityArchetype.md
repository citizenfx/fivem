---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_PORTAL_ENTITY_ARCHETYPE

```c
int GET_INTERIOR_PORTAL_ENTITY_ARCHETYPE(int interiorId, int portalIndex, int entityIndex);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)
local portalIndex = 0

if interiorId ~= 0 then
  local count = GetInteriorPortalEntityCount(interiorId, portalIndex)
  for i=0, count-1 do
    local archetype = GetInteriorPortalEntityArchetype(interiorId, portalIndex, i)
    print("portal " .. portalIndex .." entity " .. i .. " archetype is: " .. archetype)
  end
end
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.
* **entityIndex**: Portal entity index.

## Return value
Portal entity archetype.