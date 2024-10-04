---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_PORTAL_COUNT

```c
int GET_INTERIOR_PORTAL_COUNT(int interiorId);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local count = GetInteriorPortalCount(interiorId)
  print("interior " .. interiorId .. "has " .. count .. " portals")
end
```

## Parameters
* **interiorId**: The target interior.

## Return value
The amount of portals in interior.