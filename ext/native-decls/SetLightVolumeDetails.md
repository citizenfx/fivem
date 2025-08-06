---
ns: CFX
apiset: client
game: gta5
---
## SET_LIGHT_VOLUME_DETAILS

```c
void SET_LIGHT_VOLUME_DETAILS(float volIntensity, float volSizeScale, float r, float g, float b, float i, float outerExponent);
```

Set volumetric light properties for an existing light, enabling custom volumetric effects such as fog-like glow.

## Parameters

* **volIntensity**: Intensity of the volumetric effect
* **volSizeScale**: Scale of the volumetric volume
* **r**: Red channel for volumetric outer color (0–255)
* **g**: Green channel for volumetric outer color (0–255)
* **b**: Blue channel for volumetric outer color (0–255)
* **i**: Intensity (alpha) of the volumetric outer color
* **outerExponent**: Exponent controlling falloff of the volumetric outer glow
