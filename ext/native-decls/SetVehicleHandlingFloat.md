---
ns: CFX
apiset: client
game: gta5
---
## SET_VEHICLE_HANDLING_FLOAT

```c
void SET_VEHICLE_HANDLING_FLOAT(Vehicle vehicle, char* class_, char* fieldName, float value);
```

Sets a handling override for a specific vehicle. Certain handling flags can only be set globally using `SET_HANDLING_FLOAT`, this might require some experimentation.
Example: `SetVehicleHandlingFloat(vehicle, 'CHandlingData', 'fSteeringLock', 360.0)`

## Parameters
* **vehicle**: The vehicle to set data for.
* **class_**: The handling class to set. Only "CHandlingData" is supported at this time.
* **fieldName**: The field name to set. These match the keys in `handling.meta`.
* **value**: The floating-point value to set.

