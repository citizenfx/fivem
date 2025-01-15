---
ns: CFX
apiset: server
---
## GET_PED_DEATH_STATE

```c
int GET_PED_DEATH_STATE(Ped ped);
```
Retrieves the death state of the ped.

```c
enum eDeathState
{
    DeathState_Alive = 0,   // The ped is alive
    DeathState_Dying = 1,   // The ped is in the process of dying
    DeathState_Dead = 2,    // The ped is dead
    DeathState_Max = 3      // Maximum health? maybe
};
```

## Parameters
* **ped**: The target ped.

## Return value
Returns the current death state of the ped as a value from the `eDeathState` enumeration.