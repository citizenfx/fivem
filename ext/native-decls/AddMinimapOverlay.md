---
ns: CFX
apiset: client
game: gta5
---
## ADD_MINIMAP_OVERLAY

```c
int ADD_MINIMAP_OVERLAY(char* name);
```

Loads a minimap overlay from a GFx file in the current resource.

If you need to control the depth of overlay use [`ADD_MINIMAP_OVERLAY_WITH_DEPTH`](#_0xED0935B5).

## Parameters
* **name**: The path to a `.gfx` file in the current resource. It has to be specified as a `file`.

## Return value
A minimap overlay ID.
