---
ns: CFX
apiset: client
game: gta5
---
## GET_WEAPON_ANIMATION_OVERRIDE

```c
Hash GET_WEAPON_ANIMATION_OVERRIDE(Ped ped);
```
A getter for [SET_WEAPON_ANIMATION_OVERRIDE](#_0x1055AC3A667F09D9).

## Parameters
* **ped**: The target ped.

## Return value
Returns the hash for the weapon animation override

## Examples

```lua
local weaponAnimation = GetWeaponAnimationOverride(PlayerPedId())
```
