---
ns: CFX
apiset: client
game: gta5
---
## SET_SCREEN_PED_TRANSFORM

```c
BOOL SET_SCREEN_PED_TRANSFORM(Ped ped, float offsetX, float offsetY, float offsetZ, float rotX, float rotY, float rotZ);
```

Adjusts the framing of a screen ped created with `CLONE_PED_TO_SCREEN`. The offset and rotation are applied on top of the camera framing, in the space of the internal UI scene camera (the same space the pause menu character preview uses), and are reset to zero when the screen ped is created. `RESET_SCREEN_PED_CAMERA` does not reset them.

The rotation poses the ped before the camera settings are applied: the camera keeps the focus point (see `SET_SCREEN_PED_CAMERA_FOCUS`) centred, so the ped turns around that point, and `SET_SCREEN_PED_CAMERA_ROTATION` orbits around the posed ped. Rotating around Z spins the ped in place.

The offset is applied last and is not affected by the camera settings: it moves the ped relative to the camera, so moving it towards or away from the scene camera makes it larger or smaller, and the same offset moves the ped further on screen when the camera is zoomed in. Parts of the ped that leave the screen rectangle are clipped.

## Parameters
* **ped**: The screen ped handle returned by `CLONE_PED_TO_SCREEN`.
* **offsetX**: Position offset on the X axis, in meters.
* **offsetY**: Position offset on the Y axis, in meters.
* **offsetZ**: Position offset on the Z axis, in meters.
* **rotX**: Rotation offset around the X axis, in degrees.
* **rotY**: Rotation offset around the Y axis, in degrees.
* **rotZ**: Rotation offset around the Z axis (heading), in degrees.

## Return value
`true` if the transform was updated, `false` if the ped is not a screen ped or a value is not finite.
