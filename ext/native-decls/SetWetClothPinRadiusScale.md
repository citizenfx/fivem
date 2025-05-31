---
ns: CFX
apiset: client
game: gta5
---
## SET_WET_CLOTH_PIN_RADIUS_SCALE

```c
void SET_WET_CLOTH_PIN_RADIUS_SCALE(float scale);
```

Modifies the radius scale used in the simulation of wet cloth physics.
This affects how cloth behaves when wet, changing how it sticks or reacts to movement.

## Examples

```lua
SetWetClothPinRadiusScale(1.0)
```

## Parameters
* **scale**: A value that controls the wet cloth radius scale, default: 0.3, maximum: 1.0(used for dry cloth by default), lower values increase the adhesion effect of wet cloth, making it cling more tightly to the surface.
