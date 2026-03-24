---
ns: CFX
apiset: client
game: gta5
---
## PREPARE_LIGHT

```c
void PREPARE_LIGHT(int lightType, int flags, float x, float y, float z, int r, int g, int b, float intensity);
```

Create a new light with specified type, flags, position, color, and intensity.

## Parameters

* **lightType**: The type of the light
* **flags**: Light flags
* **x**: X coordinate of the light position
* **y**: Y coordinate of the light position
* **z**: Z coordinate of the light position
* **r**: Red component of the light color (0-255)
* **g**: Green component of the light color (0-255)
* **b**: Blue component of the light color (0-255)
* **intensity**: Intensity of the light
