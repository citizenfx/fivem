---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_PORTAL_ENTITY_ROTATION

```c
void GET_INTERIOR_PORTAL_ENTITY_ROTATION(int interiorId, int portalIndex, int entityIndex, float* rotX, float* rotY, float* rotZ, float* rotW);
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.
* **entityIndex**: Portal entity index.
* **rotX**: The interior entitys X rotation
* **rotY**: The interior entitys Y rotation
* **rotZ**: The interior entitys Z rotation
* **rotW**: The interior entitys W rotation

## Return value
Portal entity rotation.

## Examples
```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)
local portalIndex = 0

if interiorId ~= 0 then
  local count = GetInteriorPortalEntityCount(interiorId, portalIndex)
  for i=0, count-1 do
    local x, y, z, w = GetInteriorPortalEntityRotation(interiorId, portalIndex, i)
    print("portal " .. portalIndex .." entity " .. i .. " rotation is: " .. vec4(x, y, z, w))
  end
end
```
