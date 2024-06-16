---
ns: CFX
apiset: client
game: gta5
---
## SET_FLY_THROUGH_WINDSCREEN_PARAMS

```c
BOOL SET_FLY_THROUGH_WINDSCREEN_PARAMS(float vehMinSpeed, float unkMinSpeed, float unkModifier, float minDamage);
```

Sets some in-game parameters which is used for checks is ped needs to fly through windscreen after a crash.

## Parameters
* **vehMinSpeed**: Vehicle minimum speed (default 35.0).
* **unkMinSpeed**: Unknown minimum speed (default 40.0).
* **unkModifier**: Unknown modifier (default 17.0).
* **minDamage**: Minimum damage (default 2000.0).

## Return value
A bool indicating if parameters was set successfully.
