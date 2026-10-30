---
ns: CFX
apiset: client
game: gta5
---
## SET_SCREEN_PED_CAMERA_ZOOM

```c
BOOL SET_SCREEN_PED_CAMERA_ZOOM(Ped ped, float zoom);
```

Scales the automatic framing of a screen ped created with `CLONE_PED_TO_SCREEN` by moving its camera closer to or further from the focus point (see `SET_SCREEN_PED_CAMERA_FOCUS`).

At `1.0` (the default) the whole ped fits inside the rectangle; `2.0` puts the camera at half the distance, so the ped is drawn twice as large and only half of the full body framing fits; `0.5` draws it at half the size. Parts of the ped that leave the rectangle are clipped. The framing is recalculated when the rectangle or the field of view changes, so the zoom stays relative to the automatic fit.

The zoom is reset when the screen ped is created and by `RESET_SCREEN_PED_CAMERA`.

## Parameters
* **ped**: The screen ped handle returned by `CLONE_PED_TO_SCREEN`.
* **zoom**: Zoom factor, clamped to 0.25 - 10.0.

## Return value
`true` if the zoom was updated, `false` if the ped is not a screen ped or the value is not finite.
