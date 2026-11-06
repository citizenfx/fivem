---
ns: CFX
apiset: client
game: gta5
---
## RESET_SCREEN_PED_CAMERA

```c
BOOL RESET_SCREEN_PED_CAMERA(Ped ped);
```

Resets the camera of a screen ped created with `CLONE_PED_TO_SCREEN` to the default automatic full body framing: the field of view, rotation, focus and zoom set with `SET_SCREEN_PED_CAMERA_FOV`, `SET_SCREEN_PED_CAMERA_ROTATION`, `SET_SCREEN_PED_CAMERA_FOCUS` and `SET_SCREEN_PED_CAMERA_ZOOM`. The rectangle and the transform from `SET_SCREEN_PED_TRANSFORM` are kept.

## Parameters
* **ped**: The screen ped handle returned by `CLONE_PED_TO_SCREEN`.

## Return value
`true` if the camera was reset, `false` if the ped is not a screen ped.
