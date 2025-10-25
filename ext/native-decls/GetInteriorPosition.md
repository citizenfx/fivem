---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_POSITION

```c
void GET_INTERIOR_POSITION(int interiorId, float* posX, float* posY, float* posZ);
```

## Parameters
* **interiorId**: The target interior.
* **posX**: The X position of the current interior
* **posY**: The Y position of the current interior
* **posZ**: The Z position of the current interior

## Return value
Returns the position of the specified interior.

## Examples
```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local x, y, z = GetInteriorPosition(interiorId)
  print("current interior " .. interiorId .. " position is: " .. vec(x, y, z))
end
```
