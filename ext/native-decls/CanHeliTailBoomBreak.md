---
ns: CFX
apiset: server
game: gta5
---
## IS_HELI_TAIL_BOOM_BREAKABLE

```c
BOOL IS_HELI_TAIL_BOOM_BREAKABLE(Vehicle heli);
```

This is a getter for [SET_HELI_TAIL_EXPLODE_THROW_DASHBOARD](#_0x3EC8BF18AA453FE9)

## Parameters
* **heli**: The helicopter to check

## Return value
Returns `true` if the helicopter's tail boom can break, `false` if it cannot.