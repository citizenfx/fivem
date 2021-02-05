---
ns: CFX
apiset: client
---
## SET_DEFAULT_VEHICLE_DENSITY_MULTIPLIER

```c
void SET_DEFAULT_VEHICLE_DENSITY_MULTIPLIER(float multiplier);
```

Sets the vehicle density multiplier that will be set every tick by the game. This value is not limited to 1.0 and defines the maximum value that can be set by [SET_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME](#_0x245A6883D966D537)

## Parameters
* **multiplier**: The value to set. Default is 1.0.