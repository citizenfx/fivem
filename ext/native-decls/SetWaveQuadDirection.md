---
ns: CFX
apiset: client
game: gta5
---
## SET_WAVE_QUAD_DIRECTION

```c
BOOL SET_WAVE_QUAD_DIRECTION(int waveQuad, float directionX, float directionY);
```
directionX/Y should be constrained between -1.0 and 1.0
A positive value will create the wave starting at min and rolling towards max
A negative value will create the wave starting at max and rolling towards min
Applying both values allows you to make diagonal waves

## Examples

```lua
local success = SetWaveQuadDirection(0, 0.3, 0.1)
```


## Parameters
* **waveQuad**: The wave quad index
* **directionX**: The minX coordinate
* **directionY**: The minY coordinate

## Return value
Returns true on success.