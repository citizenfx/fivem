---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_HEAD_OVERLAY_DATA

```c
BOOL GET_PED_HEAD_OVERLAY_DATA(Ped ped, int index, int* overlayValue, int* colourType, int* firstColour, int* secondColour, float* overlayOpacity);
```

A getter for [SET_PED_HEAD_OVERLAY](#_0x48F44967FA05CC1E) and [_SET_PED_HEAD_OVERLAY_COLOR](#_0x497BF74A7B9CB952) natives.

## Examples

```lua
-- getting beard overlay data
local success, overlayValue, colourType, firstColour, secondColour, overlayOpacity = GetPedHeadOverlayData(PlayerPedId(), 1)
if success then
  -- incrementing value
  SetPedHeadOverlay(PlayerPedId(), 1, overlayValue + 1, overlayOpacity)
end
```

## Parameters
* **ped**: The target ped
* **index**: Overlay index
* **overlayValue**: Overlay value pointer
* **colourType**: Colour type pointer
* **firstColour**: First colour pointer
* **secondColour**: Second colour pointer
* **overlayOpacity**: Opacity pointer

## Return value
Returns ped's head overlay data.
