---
ns: CFX
apiset: client
---
## SET_DEFAULT_PARKED_VEHICLE_DENSITY_MULTIPLIER

```c
void SET_DEFAULT_PARKED_VEHICLE_DENSITY_MULTIPLIER(float multiplier);
```

Sets the parked vehicle density multiplier that will be set every tick by the game. This value is not limited to 1.0 and defines the maximum value that can be set by [SET_PARKED_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME](#_0xEAE6DCC7EEE3DB1D)

## Parameters
* **multiplier**: The value to set. Default is 1.0.
