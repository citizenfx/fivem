---
ns: CFX
apiset: client
game: gta5
---
## SET_LIGHT_SPECULAR_FADE_DISTANCE

```c
bool SET_LIGHT_SPECULAR_FADE_DISTANCE(int lightIndex, int fadeDistance);
```

Set the specular fade distance for a created light.

## Parameters

* **lightIndex**: The index of the created light
* **fadeDistance**: The distance at which specular highlights fade

## Return value
A boolean indicating success (`true`) or failure (`false`).
