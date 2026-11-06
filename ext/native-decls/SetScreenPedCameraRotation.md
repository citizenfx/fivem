---
ns: CFX
apiset: client
game: gta5
---
## SET_SCREEN_PED_CAMERA_ROTATION

```c
BOOL SET_SCREEN_PED_CAMERA_ROTATION(Ped ped, float pitch, float roll, float yaw);
```

Orbits the camera of a screen ped created with `CLONE_PED_TO_SCREEN` around its focus point (see `SET_SCREEN_PED_CAMERA_FOCUS`). The focus point stays in the centre of the rectangle at the same distance from the camera; `0.0, 0.0, 0.0` is the default front view.

The rotations are applied in the order yaw, pitch, roll:
* **yaw** orbits the camera around the ped's vertical axis. Positive values move the camera to the right (counter-clockwise when seen from above), showing more of the ped's left side.
* **pitch** raises or lowers the camera. Positive values look down at the ped from above, negative values look up at it from below.
* **roll** turns the camera around its viewing direction. Positive values turn the camera clockwise, so the picture turns counter-clockwise.

The scene lights stay fixed to the camera, like a photo studio with a turntable: the ped is turned in front of the lights, so it stays lit from the camera's side whichever way you orbit (a yaw of 180 shows a lit back).

The orbit is applied after the rotation from `SET_SCREEN_PED_TRANSFORM`, which poses the ped first. The offset from `SET_SCREEN_PED_TRANSFORM` is applied last and is not rotated. The rotation is reset when the screen ped is created and by `RESET_SCREEN_PED_CAMERA`.

## Parameters
* **ped**: The screen ped handle returned by `CLONE_PED_TO_SCREEN`.
* **pitch**: Rotation around the camera's horizontal axis, in degrees.
* **roll**: Rotation around the camera's viewing direction, in degrees.
* **yaw**: Rotation around the ped's vertical axis, in degrees.

## Return value
`true` if the rotation was updated, `false` if the ped is not a screen ped or a value is not finite.
