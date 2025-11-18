---
ns: CFX
apiset: client
game: gta5
---
## GET_INTERIOR_ROTATION

```c
void GET_INTERIOR_ROTATION(int interiorId, float* rotX, float* rotY, float* rotZ, float* rotW);
```


## Parameters
* **interiorId**: The target interior.
* **rotX**: The interiors X rotation
* **rotY**: The interiors Y rotation
* **rotZ**: The interiors Z rotation
* **rotW**: The interiors W rotation

## Return value
Interior rotation in quaternion.

## Examples
```lua
local playerPed = PlayerPedId()
local interiorId = GetInteriorFromEntity(playerPed)

if interiorId ~= 0 then
  local x, y, z, w = GetInteriorRotation(interiorId)
  print("current interior " .. interiorId .. " rotation is: " .. vec(x, y, z, w))
end
```
