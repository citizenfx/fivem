---
ns: CFX
apiset: client
game: gta5
---
## REMOVE_MINIMAP_OVERLAY

```c
void REMOVE_MINIMAP_OVERLAY(int miniMap);
```

Removes a minimap overlay added with [`ADD_MINIMAP_OVERLAY`](#_0x4AFD2499) or
[`ADD_MINIMAP_OVERLAY_WITH_DEPTH`](#_0xED0935B5).

Overlays are removed automatically when the resource that added them stops, so this is only needed
to remove one before that. Passing an ID that is unknown, or that has already been removed, does
nothing.

## Parameters
* **miniMap**: The minimap overlay ID.
