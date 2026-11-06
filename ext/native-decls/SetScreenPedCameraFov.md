---
ns: CFX
apiset: client
game: gta5
---
## SET_SCREEN_PED_CAMERA_FOV

```c
BOOL SET_SCREEN_PED_CAMERA_FOV(Ped ped, float fov);
```

Sets the field of view of the camera that draws a screen ped created with `CLONE_PED_TO_SCREEN`. Every screen ped has its own camera settings, so screen peds shown at the same time can use different values.

The field of view works like a camera lens: low values (telephoto) flatten the ped, high values (wide angle) exaggerate the perspective, for example a big head and small feet in a low-angle shot. The automatic framing is recalculated for the new field of view (the camera moves closer or further away), so the ped keeps the same size inside its rectangle and only the perspective changes. Use `SET_SCREEN_PED_CAMERA_ZOOM` to change the size.

The value is measured across half the screen height (the size of the internal render buffer), like the pause menu character preview, which uses 40 degrees. It is reset when the screen ped is created and by `RESET_SCREEN_PED_CAMERA`.

Very wide angles combined with a high zoom can put the camera inside the ped.

## Parameters
* **ped**: The screen ped handle returned by `CLONE_PED_TO_SCREEN`.
* **fov**: Field of view in degrees, clamped to 5.0 - 120.0. The default is the field of view of the internal UI scene camera (40.0).

## Return value
`true` if the field of view was updated, `false` if the ped is not a screen ped or the value is not finite.
