---
ns: CFX
apiset: client
game: gta5
---
## IS_PED_COMPONENT_VARIATION_GEN9_EXCLUSIVE

```c
bool IS_PED_COMPONENT_VARIATION_GEN9_EXCLUSIVE(Ped ped, int componentId, int drawableId);
```

## Examples
```lua
local ped = PlayerPedId()

for component = 0, 12 do
  local count = GetNumberOfPedDrawableVariations(ped, component)

  for drawable = 0, count - 1 do
    if IsPedComponentVariationGen9Exclusive(ped, component, drawable) then
      print("Component " .. component .. " drawable " .. drawable .. " is a gen9 exclusive, skip!")
    end
  end
end
```

## Parameters
* **ped**: The target ped.
* **componentId**: The component id.
* **drawableId**: The drawable id.

## Return value
Whether or not the ped component variation is a gen9 exclusive (stub assets).
