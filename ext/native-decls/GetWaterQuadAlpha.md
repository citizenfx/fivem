---
ns: CFX
apiset: client
game: gta5
---
## GET_WATER_QUAD_ALPHA

```c
BOOL GET_WATER_QUAD_ALPHA(int waterQuad, int* a0, int* a1, int* a2, int* a3);
```

## Parameters
* **waterQuad**: The water quad index
* **a0**: The a0 level
* **a1**: The a1 level
* **a2**: The a2 level
* **a3**: The a3 level

## Return value
Returns `true` if the operation was successful, `false` otherwise, the bounds will be set to `0` on failure.

## Examples

```lua
local success, a0, a1, a2, a3 = GetWaterQuadAlpha(0)
```
