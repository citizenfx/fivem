---
ns: CFX
apiset: client
game: gta5
---
## SET_COLSHAPE_SYSTEM_UPDATE_INTERVAL

```c
void SET_COLSHAPE_SYSTEM_UPDATE_INTERVAL(int updateInterval);
```

Sets the internal collision detection systems update interval. The standard time for this is 100ms. For faster collision detection you can decrease this value.

## Parameters
* **updateInterval**: the update time in milliseconds, minimum is 0ms (every tick)