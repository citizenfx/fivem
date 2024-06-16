---
ns: CFX
apiset: client
game: gta5
---
## GET_CALMING_QUAD_DAMPENING

```c
BOOL GET_CALMING_QUAD_DAMPENING(int waterQuad, float* calmingQuadDampening);
```

## Examples

```lua
local success, dampening = GetCalmingQuadDampening(1)
```

## Parameters
* **waterQuad**: The calming quad index

## Return value
Returns true on success. Dampening value is undefined on failure