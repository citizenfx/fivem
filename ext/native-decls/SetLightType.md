---
ns: CFX
apiset: client
game: gta5
---
## SET_LIGHT_TYPE

```c
bool SET_LIGHT_TYPE(int lightIndex, int lightType);
```

Change the light type of a already created light.
Certain light type needs more configurations to work properly (Like direction, flags or size)

## Parameters
* **lightIndex**: The index of the created light
* **lightType**: The type of light

```cpp
enum eLightType
{
    NOTHING = 0,
    POINT = 1,
    SPOT = 2,
    CAPSULE = 4,
    DIRECTION = 8,
    AO = 16,
}
```

## Return value
A boolean indicating whether the operation succeeded.

