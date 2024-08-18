---
ns: CFX
apiset: client
game: gta5
---
## SET_WATER_AREA_CLIP_RECT

```c
void SET_WATER_AREA_CLIP_RECT(int minX, int minY, int maxX, int maxY);
```

Sets world clip boundaries for water quads file (water.xml, water_heistisland.xml)
Used internally by LOAD_GLOBAL_WATER_FILE

## Examples

```lua
SetWaterAreaClipRect(-4000, -4000, 4500, 8000)
```

## Parameters
* **minX**: 
* **minY**: 
* **maxX**: 
* **maxY**: 

```
Supported Builds: 2189+
```
