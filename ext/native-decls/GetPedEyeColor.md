---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_EYE_COLOR

```c
int GET_PED_EYE_COLOR(Ped ped);
```

A getter for [_SET_PED_EYE_COLOR](#_0x50B56988B170AFDF). Returns -1 if fails to get.

## Examples

```lua
local pedEyeColour = GetPedEyeColor(PlayerPedId())
if pedEyeColour == 7 then
  print("Gray eyes!")
end
```

## Parameters
* **ped**: The target ped

## Return value
Returns ped's eye colour, or -1 if fails to get.
