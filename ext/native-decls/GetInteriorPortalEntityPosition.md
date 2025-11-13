---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_PORTAL_ENTITY_POSITION

```c
void GET_INTERIOR_PORTAL_ENTITY_POSITION(int interiorId, int portalIndex, int entityIndex, float* posX, float* posY, float* posZ);
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.
* **entityIndex**: Portal entity index.
* **posX**: the x position of the entity
* **posY**: the y position of the entity
* **posZ**: the z position of the entity

## Return value
Portal entity position.

## Examples
```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)
local portalIndex = 0

if interiorId ~= 0 then
  local count = GetInteriorPortalEntityCount(interiorId, portalIndex)
  for i=0, count-1 do
    local x, y, z = GetInteriorPortalEntityPosition(interiorId, portalIndex, i)
    print("portal " .. portalIndex .." entity " .. i .. " position is: " .. vec3(x, y, z))
  end
end
```
