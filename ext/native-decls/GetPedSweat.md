---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_SWEAT

```c
float GET_PED_SWEAT(Ped ped);
```
A getter for [SET_PED_SWEAT](#_0x27B0405F59637D1F).

## Examples

```lua
local sweat = GetPedSweat(PlayerPedId())
```

## Parameters
* **ped**: The target ped

## Return value
Returns ped's sweat.