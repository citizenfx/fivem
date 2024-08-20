---
ns: CFX
apiset: server
game: gta5
---
## GET_HELI_DISABLE_EXPLODE_FROM_BODY_DAMAGE

```c
BOOL GET_HELI_DISABLE_EXPLODE_FROM_BODY_DAMAGE(Vehicle heli);
```

This is a getter for [SET_DISABLE_HELI_EXPLODE_FROM_BODY_DAMAGE](#_0xEDBC8405B3895CC9)

## Parameters
* **heli**: The helicopter to check

## Return value
Returns `true` if the helicopter is set to be protected from exploding due to minor body damage, `false` otherwise.