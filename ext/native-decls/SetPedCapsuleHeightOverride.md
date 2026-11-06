---
ns: CFX
apiset: client
game: gta5
---
## SET_PED_CAPSULE_HEIGHT_OVERRIDE

```c
void SET_PED_CAPSULE_HEIGHT_OVERRIDE(Ped ped, float height);
```

Overrides the height of the ped's capsule, which is used for collision. Lowering it allows the ped to traverse under
obstacles it would otherwise be too tall for. This override is synchronised to other players if set by the network
owner. This does not impact weapon damage, such as shooting the ped.

To clear the override, use [CLEAR_PED_CAPSULE_HEIGHT_OVERRIDE](#_0x85A6FDA3).

## Parameters
* **ped**: The ped to change.
* **height**: The top of the capsule, in meters above the ped's position. The default is about `0.82`, and must be greater than `0.0`.

## Examples
```lua
local ped = PlayerPedId()

-- crouch: let the ped fit under low obstacles
SetPedCapsuleHeightOverride(ped, 0.25)

-- stand back up, restoring the default height
ClearPedCapsuleHeightOverride(ped)
```
