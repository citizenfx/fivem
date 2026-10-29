---
ns: CFX
apiset: client
game: gta5
---
## SET_PED_CAPSULE_HEIGHT_OVERRIDE

```c
void SET_PED_CAPSULE_HEIGHT_OVERRIDE(Ped ped, float height);
```

Overrides the height of a ped's mover capsule, the collision shape the game uses to move the ped through the world.
Lowering it lets the ped move under obstacles it would otherwise collide with, for example a crouching ped walking
under a trailer.

The height is the top of the capsule, in meters above the ped's position (as returned by `GET_ENTITY_COORDS`). For
regular human peds the game's default is `0.82`, and the game's own (otherwise unused) crouched height is `0.25`. The
bottom of the capsule isn't changed.

The override stays in place until it is removed with [CLEAR_PED_CAPSULE_HEIGHT_OVERRIDE](#_0x85A6FDA3), the ped is
deleted or the player leaves the server. Removing it while there's an obstacle above the ped will push the ped out of
it, so scripts should check for headroom before standing a ped back up.

The override is synchronized to other players along with the ped, so they collide with the lowered capsule as well
(on servers that support it). For peds owned by another player, their override is used, and any override set locally
is replaced when the ped is next synchronized. It doesn't change hit detection for weapons, which uses the ped's
skeleton.

## Parameters
* **ped**: The ped to change.
* **height**: The height of the top of the capsule above the ped's position, in meters. Must be greater than `0.0`.

## Examples
```lua
local ped = PlayerPedId()

-- crouch: let the ped fit under low obstacles
SetPedCapsuleHeightOverride(ped, 0.25)

-- stand back up, restoring the default height
ClearPedCapsuleHeightOverride(ped)
```
