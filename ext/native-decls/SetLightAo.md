---
ns: CFX
apiset: client
game: gta5
---
## SET_LIGHT_AO

```c
bool SET_LIGHT_AO(int lightIndex, float intensity, float radius, float bias, float intensity2);
```

Set ambient occlusion (AO) parameters for a specified light.

## Parameters

* **lightIndex**: The index of the light
* **intensity**: The AO intensity
* **radius**: The AO radius
* **bias**: The AO bias
* **intensity2**: Secondary AO intensity

## Return value
A boolean indicating whether the operation succeeded.
