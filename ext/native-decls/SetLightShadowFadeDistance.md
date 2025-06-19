---
ns: CFX
apiset: client
game: gta5
---
## SET_LIGHT_SHADOW_FADE_DISTANCE

```c
bool SET_LIGHT_SHADOW_FADE_DISTANCE(int lightIndex, int fadeDistance);
```

Set the fade distance for the shadows of a created light.

## Parameters

* **lightIndex**: The index of the created light
* **fadeDistance**: The distance at which the shadow fades

## Return value
A boolean indicating success (`true`) or failure (`false`).
