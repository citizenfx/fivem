---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_PORTAL_CORNER_POSITION

```c
void GET_INTERIOR_PORTAL_CORNER_POSITION(int interiorId, int portalIndex, int cornerIndex, float* posX, float* posY, float* posZ);
```

## Parameters
* **interiorId**: The target interior.
* **portalIndex**: Interior portal index.
* **cornerIndex**: Portal's corner index, each portal will always have `4` corner indexs.
* **posX**: The X position of the specified corner
* **posY**: The Y position of the specified corner
* **posZ**: The Z position of the specified corner

## Return value
Returns the position of the specified `cornerIndex`

## Examples
```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local portalIndex = 0

  for i = 0, 3 do
    local x, y, z = GetInteriorPortalCornerPosition(interiorId, portalIndex, i)
    print("position of portal " .. portalIndex .. "corner index " .. i .. " is: " .. vec(x, y, z))
  end
end
```
