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
--[[ 
  Example =>
     Using GetAllPeds, This example prints information of all peds, Ped id, Owner, Model and Armour.
]]

local allPeds = GetAllPeds() -- Get all peds handles known to the server.
for i, ped in ipairs(allPeds) do
    local pedOwner = NetworkGetEntityOwner(ped)
    if pedOwner ~= nil and pedOwner then
    local playerName = GetPlayerName(pedOwner)
    local pedModel = GetEntityModel(ped)
    local pedArmour = GetPedArmour(ped)
    print("Ped : "..ped.." | Owner name : "..playerName.." | Model : "..pedModel.." | Armour : "..pedArmour)
end
```
