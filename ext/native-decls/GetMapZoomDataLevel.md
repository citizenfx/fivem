---
ns: CFX
apiset: client
game: gta5
---
## GET_MAP_ZOOM_DATA_LEVEL

```c
BOOL GET_MAP_ZOOM_DATA_LEVEL(int index, float* zoomScale, float* zoomSpeed, float* scrollSpeed, float* tilesX, float* tilesY);
```

Returns the zoom level data by index from mapzoomdata.meta file.

## Parameters
* **index**: Zoom level index.
* **zoomScale**: fZoomScale value.
* **zoomSpeed**: fZoomSpeed value.
* **scrollSpeed**: fScrollSpeed value.
* **tilesX**: vTiles X.
* **tilesY**: vTiles Y.

## Return value
A boolean indicating TRUE if the data was received successfully.
