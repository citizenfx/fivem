---
ns: CFX
apiset: client
game: gta5
---
## SET_WATER_QUAD_ALPHA

```c
BOOL SET_WATER_QUAD_ALPHA(int waterQuad, int a0, int a1, int a2, int a3);
```
## Examples

```lua
local success = SetWaterQuadAlpha(0, 5, 5, 5, 5)
```

## Parameters
* **waterQuad**: The water quad index
* **a0**: The a0 level
* **a1**: The a1 level
* **a2**: The a2 level
* **a3**: The a3 level

## Return value
Returns true on success.