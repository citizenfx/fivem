---
ns: CFX
apiset: client
game: gta5
---
## GET_CALMING_QUAD_DAMPENING

```c
BOOL GET_CALMING_QUAD_DAMPENING(int waterQuad, float* calmingQuadDampening);
```

## Parameters
* **waterQuad**: The calming quad index
* **calmingQuadDampening**: 

## Return value
Returns `true` on success. `calmingQuadDampening` will be set to -1.0 on failure.

## Examples

```lua
local success, dampening = GetCalmingQuadDampening(1)
```
