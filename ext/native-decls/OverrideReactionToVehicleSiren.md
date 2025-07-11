---
ns: CFX
apiset: client
game: gta5
---
## OVERRIDE_REACTION_TO_VEHICLE_SIREN

```c
void OVERRIDE_REACTION_TO_VEHICLE_SIREN(BOOL state, int reaction);
```

Setting the state to true and a value between 0 and 2 will cause pedestrian vehicles to react accordingly to sirens.

```c
enum Reactions {
    Left = 0,
    Right = 1,
    Stop = 2
}
```

## Parameters
* **state**: Toggle on or off
* **reaction**: Decide how they should react