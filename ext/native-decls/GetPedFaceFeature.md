---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_FACE_FEATURE

```c
float GET_PED_FACE_FEATURE(Ped ped, int index);
```

A getter for [_SET_PED_FACE_FEATURE](#_0x71A5C1DBA060049E). Returns 0.0 if fails to get.

## Examples

```lua
local noseWidth = GetPedFaceFeature(PlayerPedId(), 0)
if noseWidth == 1.0 then
  print("You have big nose!")
end
```

## Parameters
* **ped**: The target ped
* **index**: Face feature index

## Return value
Returns ped's face feature value, or 0.0 if fails to get.
