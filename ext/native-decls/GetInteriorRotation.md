---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_ROTATION

```c
void GET_INTERIOR_ROTATION(int interiorId, float* rotx, float* rotY, float* rotZ, float* rotW);
```

## Examples

```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local x, y, z, w = GetInteriorRotation(interiorId)
  print("current interior " .. interiorId .. " rotation is: " .. vec(x, y, z, w))
end
```

## Parameters
* **interiorId**: The target interior.

## Return value
Interior rotation in quaternion.