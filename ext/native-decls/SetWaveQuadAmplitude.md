---
ns: CFX
apiset: client
game: gta5
---
## SET_WAVE_QUAD_AMPLITUDE

```c
BOOL SET_WAVE_QUAD_AMPLITUDE(int waveQuad, float amplitude);
```

## Parameters
* **waveQuad**: The wave quad index
* **amplitude**: The amplitude value

## Return value
Returns `true` if `waveQuad`s amplitude got set, `false` otherwise.

## Examples
```lua
local success = SetWaveQuadAmplitude(0, 1.0)
```
