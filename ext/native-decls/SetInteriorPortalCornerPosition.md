---
ns: CFX
apiset: client
game: gta5
---
## SET_INTERIOR_PORTAL_CORNER_POSITION

```c
void SET_INTERIOR_PORTAL_CORNER_POSITION(int interiorId, int portalIndex, int cornerIndex, float posX, float posY, float posZ);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local portalCount = GetInteriorPortalCount(interiorId)

  -- rip portals
  for portalIndex = 0, portalCount - 1 do
    for cornerIndex = 0, 3 do -- 4 corners
      SetInteriorPortalCornerPosition(interiorId, portalIndex, cornerIndex, 0.0, 0.0, 0.0)
    end
  end
  
  RefreshInterior(interiorId)
end
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.
* **cornerIndex**: Interior corner index.
* **posX**:
* **posY**:
* **posZ**:
