---
ns: CFX
apiset: client
game: gta5
---
## SET_PED_MELEE_COMBAT_LIMITS

```c
void SET_PED_MELEE_COMBAT_LIMITS(int primaryCount, int secondaryCount, int populationPedCount);
```

Override the limits on the number and types of melee combatants. The game is limited to at most ten combatants among the three types: primary, secondary, and observers.

This native infers the number of observers based on the primary and secondary counts.

## Parameters
* **primaryCount**: The number of peds that engage in combat (default: 1)
* **secondaryCount**: The number of peds that engage in taunting (default: 3)
* **populationPedCount**: The maximum number of population peds (ambient and scenario) that can engage in combat (default: 3)
