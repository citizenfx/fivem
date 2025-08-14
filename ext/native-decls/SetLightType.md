---
ns: CFX
apiset: client
game: gta5
---
## SET_LIGHT_TYPE

```c
void SET_LIGHT_TYPE(int lightType);
```

Change the light type of a already created light.
Certain light type needs more configurations to work properly (Like direction, flags or size)

## Parameters

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
