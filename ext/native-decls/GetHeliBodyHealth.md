---
ns: CFX
apiset: server
game: gta5
---
## GET_HELI_BODY_HEALTH

```c
int GET_HELI_BODY_HEALTH(Vehicle heli);
```

**Note** This native will always return `1000.0` unless [SET_VEHICLE_BODY_HEALTH](#_0xB77D05AC8C78AADB), [SET_VEHICLE_ENGINE_HEALTH](#_0x45F6D8EEF34ABEF1), or [SET_VEHICLE_PETROL_TANK_HEALTH](#_0x70DB57649FA8D0D8) have been called with a value greater than `1000.0`.

## Parameters
* **heli**: The helicopter to check

## Return value
Returns the current health of the helicopter's body.