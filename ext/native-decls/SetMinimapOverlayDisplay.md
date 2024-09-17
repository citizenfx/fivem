---
ns: CFX
apiset: client
game: gta5
---
## SET_MINIMAP_OVERLAY_DISPLAY

```c
void SET_MINIMAP_OVERLAY_DISPLAY(int miniMap, float x, float y, float xScale, float yScale, float alpha);
```

Sets the display info for a minimap overlay.

## Parameters
* **miniMap**: The minimap overlay ID.
* **x**: The X position for the overlay. This is equivalent to a game coordinate X.
* **y**: The Y position for the overlay. This is equivalent to a game coordinate Y, except that it's inverted (gfxY = -gameY).
* **xScale**: The X scale for the overlay. This is equivalent to the Flash _xscale property, therefore 100 = 100%.
* **yScale**: The Y scale for the overlay. This is equivalent to the Flash _yscale property.
* **alpha**: The alpha value for the overlay. This is equivalent to the Flash _alpha property, therefore 100 = 100%.

