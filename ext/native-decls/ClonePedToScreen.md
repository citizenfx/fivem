---
ns: CFX
apiset: client
game: gta5
---
## CLONE_PED_TO_SCREEN

```c
Ped CLONE_PED_TO_SCREEN(Ped ped, float x, float y, float width, float height);
```

Clones a ped (like `CLONE_PED`) and renders the clone as a live, lit 2D overlay inside the given screen rectangle, using the same renderer as the pause menu character preview. This works without the pause menu being open, and the clone is drawn every frame above NUI windows (but below the NUI cursor).

The clone is a regular, non-networked script entity owned by the calling resource: you can animate it (e.g. `TASK_PLAY_ANIM`) or change its components, props and head blend like any other ped. It is frozen, has no collision, is invincible and is hidden from the game world - it is only visible on screen.

The whole ped (head to feet, with a small margin) is scaled to fit inside the rectangle and centred in it; the rectangle also clips the ped. The renderer draws into a buffer of half the screen size, so at most about `aspect ratio / 2` of the screen height (0.89 on 16:9) and half the screen height in width are used for the ped, centred in larger rectangles. Use the `SET_SCREEN_PED_CAMERA_*` natives (field of view, rotation, focus and zoom, for example a headshot portrait) and `SET_SCREEN_PED_TRANSFORM` to adjust the framing; every screen ped has its own settings.

Notes:
* At most 5 screen peds can exist at the same time (across all resources).
* The pause menu and game scripts using the `UI3DSCENE_*` natives take priority: while either is using the renderer (for example while the pause menu is open), screen peds are not drawn.
* Screen peds are removed when the owning resource stops or when the network session ends.
* Use `DELETE_SCREEN_PED` to remove a screen ped; deleting the clone with `DELETE_ENTITY` also removes it from the screen.

## Parameters
* **ped**: The ped to clone.
* **x**: Left edge of the rectangle, normalized screen coordinate (0.0 = left edge of the screen, 1.0 = right edge).
* **y**: Top edge of the rectangle, normalized screen coordinate (0.0 = top edge of the screen, 1.0 = bottom edge).
* **width**: Width of the rectangle, normalized to the screen width (must be greater than 0.0).
* **height**: Height of the rectangle, normalized to the screen height (must be greater than 0.0).

## Return value
The handle of the cloned ped, or 0 if the ped couldn't be cloned, the rectangle is invalid, or all 5 screen ped slots are in use.
