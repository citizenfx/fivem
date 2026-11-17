---
ns: CFX
apiset: client
game: gta5
---
## SET_SCREEN_PED_CAMERA_FOCUS

```c
BOOL SET_SCREEN_PED_CAMERA_FOCUS(Ped ped, float offsetX, float offsetY, float offsetZ);
```

Sets the point on a screen ped created with `CLONE_PED_TO_SCREEN` that the camera frames: this point is drawn in the centre of the rectangle, and `SET_SCREEN_PED_CAMERA_ROTATION` orbits around it.

The point is given in the ped's own coordinates, relative to the ped's origin (around the pelvis): X is the ped's right, Y its front and Z up, in meters. The default `0.0, 0.0, -0.1` is the centre of the automatic full body framing (which spans from 1.2 below to 1.0 above the origin). The head of a standard ped is roughly 0.65 above the origin; for exact values, get a bone position on the original ped with `GET_PED_BONE_COORDS` and convert it with `GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS`.

The focus only moves the framing; it does not change the size of the ped. Combine it with `SET_SCREEN_PED_CAMERA_ZOOM` for close-ups, for example a headshot portrait:

```lua
local portrait = ClonePedToScreen(PlayerPedId(), 0.02, 0.05, 0.08, 0.14)

SetScreenPedCameraFocus(portrait, 0.0, 0.0, 0.65) -- the head
SetScreenPedCameraZoom(portrait, 4.0) -- about 0.55 m of the ped fits the rectangle
SetScreenPedCameraFov(portrait, 25.0) -- flatter, more flattering perspective
```

The focus follows the rotation from `SET_SCREEN_PED_TRANSFORM`, so a posed or turning ped keeps its focus point centred. The focus is reset when the screen ped is created and by `RESET_SCREEN_PED_CAMERA`.

## Parameters
* **ped**: The screen ped handle returned by `CLONE_PED_TO_SCREEN`.
* **offsetX**: Offset to the ped's right, in meters (clamped to -10.0 - 10.0).
* **offsetY**: Offset to the ped's front, in meters (clamped to -10.0 - 10.0).
* **offsetZ**: Offset upwards, in meters (clamped to -10.0 - 10.0).

## Return value
`true` if the focus was updated, `false` if the ped is not a screen ped or a value is not finite.
