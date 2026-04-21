---
ns: CFX
apiset: client
game: gta5
---
## GET_WATER_QUAD_IS_INVISIBLE

```c
BOOL GET_WATER_QUAD_IS_INVISIBLE(int waterQuad, int* isInvisible);
```

## Parameters
* **waterQuad**: The water quad index
* **isInvisible**: Gets set to `1` if the quad is invisible.

## Return value
Returns `true` if the operation was successful, `false` otherwise, `isInvisible` will be set to `0` on failure.

## Examples
```lua
local success, isInvisible = GetWaterQuadIsInvisible(0)
```
