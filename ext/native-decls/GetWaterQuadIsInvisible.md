---
ns: CFX
apiset: client
game: gta5
---
## GET_WATER_QUAD_IS_INVISIBLE

```c
BOOL GET_WATER_QUAD_IS_INVISIBLE(int waterQuad, int* isInvisible);
```
## Examples

```lua
local success, isInvisible = GetWaterQuadIsInvisible(0)
```

## Parameters
* **waterQuad**: The water quad index

## Return value
Returns if the given water quad is invisible