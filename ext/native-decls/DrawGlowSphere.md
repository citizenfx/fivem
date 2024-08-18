---
ns: CFX
apiset: client
game: gta5
---
## DRAW_GLOW_SPHERE

```c
void DRAW_GLOW_SPHERE(float posX, float posY, float posZ, float radius, int colorR, int colorG, int colorB, float intensity, BOOL invert, BOOL marker);
```

Draw a glow sphere this frame. Up to 256 per single frame.

## Parameters
* **posX**: Position X.
* **posY**: Position Y.
* **posZ**: Position Z.
* **radius**: Sphere radius.
* **colorR**: Red.
* **colorG**: Green.
* **colorB**: Blue.
* **intensity**: Intensity.
* **invert**: Invert rendering.
* **marker**: Draw as a marker, otherwise as an overlay.
