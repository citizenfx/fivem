---
ns: CFX
apiset: server
---
## GET_ALL_PEDS

```c
object GET_ALL_PEDS();
```

Returns all peds handles known to the server.
The data returned adheres to the following layout:
```
[127, 42, 13, 37]
```

## Return value
An object containing a list of peds handles.

## Examples

```lua
-- This example prints information of every ped that has an owner.

for i, ped in ipairs(GetAllPeds()) do
    local pedOwner = NetworkGetEntityOwner(ped)
    if pedOwner > 0 then
       local playerName = GetPlayerName(pedOwner)
       local pedModel = GetEntityModel(ped)
       local pedArmour = GetPedArmour(ped)
       print("Ped : "..ped.." | Owner name : "..playerName.." | Model : "..pedModel.." | Armour : "..pedArmour)
    end
end
```
