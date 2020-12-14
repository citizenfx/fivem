---
ns: CFX
apiset: client
game: gta5
---
## OVERRIDE_VEHICLE_PEDS_CAN_STAND_ON_TOP_FLAG

```c
void OVERRIDE_VEHICLE_PEDS_CAN_STAND_ON_TOP_FLAG(Vehicle vehicle, BOOL can);
```

Overrides whether or not peds can stand on top of the specified vehicle.

Note this flag is not replicated automatically, you will have to manually do so.

## Parameters
* **vehicle**: The vehicle.
* **can**: Can they?