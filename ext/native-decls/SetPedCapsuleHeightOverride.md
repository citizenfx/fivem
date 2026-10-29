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

The override stays in place until it is removed, the ped is deleted or the player leaves the server. Removing it while
there's an obstacle above the ped will push the ped out of it, so scripts should check for headroom before standing a
ped back up.

The override only affects the local game: it doesn't change hit detection for weapons (which uses the ped's skeleton),
and it isn't synchronized to other players.

## Parameters
* **ped**: The ped to change.
* **height**: The height of the top of the capsule above the ped's position, in meters. `0.0` removes the override.

## Examples
```lua
local ped = PlayerPedId()

-- crouch: let the ped fit under low obstacles
SetPedCapsuleHeightOverride(ped, 0.25)

-- stand back up, restoring the default height
SetPedCapsuleHeightOverride(ped, 0.0)
```
