---
ns: CFX
apiset: client
game: gta5
---
## SET_SCREEN_PED_RECT

```c
BOOL SET_SCREEN_PED_RECT(Ped ped, float x, float y, float width, float height);
```

Moves and resizes the screen rectangle of a screen ped created with `CLONE_PED_TO_SCREEN`. The whole ped is scaled to fit inside the new rectangle and centred in it.

## Parameters
* **ped**: The screen ped handle returned by `CLONE_PED_TO_SCREEN`.
* **x**: Left edge of the rectangle, normalized screen coordinate (0.0 = left edge of the screen, 1.0 = right edge).
* **y**: Top edge of the rectangle, normalized screen coordinate (0.0 = top edge of the screen, 1.0 = bottom edge).
* **width**: Width of the rectangle, normalized to the screen width (must be greater than 0.0).
* **height**: Height of the rectangle, normalized to the screen height (must be greater than 0.0).

## Return value
`true` if the rectangle was updated, `false` if the ped is not a screen ped or the rectangle is invalid.
