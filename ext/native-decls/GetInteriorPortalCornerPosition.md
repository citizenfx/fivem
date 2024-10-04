---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_PORTAL_CORNER_POSITION

```c
void GET_INTERIOR_PORTAL_CORNER_POSITION(int interiorId, int portalIndex, int cornerIndex, float* posX, float* posY, float* posZ);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local portalIndex = 0
  local cornerIndex = 0

  local x, y, z = GetInteriorPortalCornerPosition(interiorId, portalIndex, cornerIndex)
  print("position of portal " .. portalIndex .. "corner index " .. cornerIndex .. " is: " .. vec(x, y, z))
end
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.
* **cornerIndex**: Portal's corner index.

## Return value
Portal corner position.