---
ns: CFX
apiset: client
game: gta5
---
## ADD_MINIMAP_OVERLAY_WITH_DEPTH

```c
int ADD_MINIMAP_OVERLAY_WITH_DEPTH(char* name, int depth, cs_split BOOL background);
```

Loads a minimap overlay from a GFx file in the current resource.

## Parameters
* **name**: The path to a `.gfx` file in the current resource. It has to be specified as a `file`.
* **depth**: The depth of new overlay on the minimap. Pass `-1` for game to figure out the highest depth itself. Should not be greater than `0x7EFFFFFD`.
* **background**: The overlay is a background overlay.

## Return value
A minimap overlay ID.
