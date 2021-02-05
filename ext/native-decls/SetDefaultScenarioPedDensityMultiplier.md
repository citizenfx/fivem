---
ns: CFX
apiset: client
---
## SET_DEFAULT_SCENARIO_PED_DENSITY_MULTIPLIER

```c
void SET_DEFAULT_SCENARIO_PED_DENSITY_MULTIPLIER(float multiplier);
```

Sets the scenario ped density multiplier that will be set every tick by the game. This value is not limited to 1.0 and defines the maximum value that can be set by [SET_SCENARIO_PED_DENSITY_MULTIPLIER_THIS_FRAME](#_0x7A556143A1C03898)

## Parameters
* **multiplier**: The value to set. Default is 1.0.