---
ns: CFX
apiset: client
game: gta5
---
## GET_WAVE_QUAD_AMPLITUDE

```c
BOOL GET_WAVE_QUAD_AMPLITUDE(int waveQuad, float* waveQuadAmplitude);
```

## Examples

```lua
local success, amplitude = GetWaveQuadAmplitude(1)
```

## Parameters
* **waveQuad**: The wave quad index

## Return value
Returns true on success. Amplitude is undefined on failure