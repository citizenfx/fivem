---
ns: CFX
apiset: client
game: gta5
---
## SET_WATER_AREA_CLIP_RECT

```c
void SET_WATER_AREA_CLIP_RECT(int minX, int minY, int maxX, int maxY);
```

**NOTE**: This native only works on build 2189+

Sets world clip boundaries for water quads file (water.xml, water_heistisland.xml)
Used internally by LOAD_GLOBAL_WATER_FILE


## Parameters
* **minX**: 
* **minY**: 
* **maxX**: 
* **maxY**: 

## Examples

```lua
SetWaterAreaClipRect(-4000, -4000, 4500, 8000)
```
