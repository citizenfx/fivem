---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_DASHBOARD_TEMP

```c
float GET_VEHICLE_DASHBOARD_TEMP();
```

You can change what this value shows by modifying [SET_VEHICLE_ENGINE_TEMPERATURE](#_0x6C93C4A9) with a value over `100.0`

## Return value
Returns a value between `0.0` and `1.0` which represents the angle of the temperature meter
