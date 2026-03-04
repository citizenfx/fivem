---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_HAIR_COLOR

```c
int GET_PED_HAIR_COLOR(Ped ped);
```

A getter for [_SET_PED_HAIR_COLOR](#_0x4CFFC65454C93A49). 

## Examples

```lua
local primaryColour = GetPedHairColor(PlayerPedId())
if primaryColour == 18 then
  print("You have red hair!")
end
```

## Parameters
* **ped**: The target ped

## Return value
Returns ped's primary hair colour, or -1 if the ped doesn't have it set.
