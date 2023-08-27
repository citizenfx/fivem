---
ns: CFX
apiset: client
game: gta5
---
## SET_CALMING_QUAD_DAMPENING

```c
BOOL SET_CALMING_QUAD_DAMPENING(int calmingQuad, float dampening);
```
## Examples

```lua
local success = SetCalmingQuadDampening(0, 1.0)
```

## Parameters
* **calmingQuad**: The calming quad
* **dampening**: The dampening value

## Return value
Returns true on success.