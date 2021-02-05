---
ns: CFX
apiset: client
---
## SET_DEFAULT_PED_DENSITY_MULTIPLIER

```c
void SET_DEFAULT_PED_DENSITY_MULTIPLIER(float multiplier);
```

Sets the ped density multiplier that will be set every tick by the game. This value is not limited to 1.0 and defines the maximum value that can be set by [SET_PED_DENSITY_MULTIPLIER_THIS_FRAME](#_0x95E3D6257B166CF2)

## Parameters
* **multiplier**: The value to set. Default is 1.0.