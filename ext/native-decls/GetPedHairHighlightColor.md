---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_HAIR_HIGHLIGHT_COLOR

```c
int GET_PED_HAIR_HIGHLIGHT_COLOR(Ped ped);
```

A getter for [_SET_PED_HAIR_COLOR](#_0x4CFFC65454C93A49). Returns -1 if fails to get.

## Examples

```lua
local secondaryColour = GetPedHairHighlightColor(PlayerPedId())
if secondaryColour == 32 then
  print("You have pink hair highlight colour!")
end
```

## Parameters
* **ped**: The target ped

## Return value
Returns ped's secondary hair colour.
