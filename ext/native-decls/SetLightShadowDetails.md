---
ns: CFX
apiset: client
game: gta5
---
## SET_LIGHT_SHADOW_DETAILS

```c
bool SET_LIGHT_SHADOW_DETAILS(int lightIndex, int shadowFlags, float shadowDistance, float shadowFade, float shadowDepthBiasScale);
```

Set the shadow details for a created light.

## Parameters

* **lightIndex**: The index of the created light
* **shadowFlags**: Flags controlling shadow behavior
* **shadowDistance**: The distance at which shadows are rendered
* **shadowFade**: The fade distance for shadows
* **shadowDepthBiasScale**: The depth bias scale

## Return value
A boolean indicating whether the operation succeeded.
