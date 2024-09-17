---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_HANDLING_FLOAT

```c
float GET_VEHICLE_HANDLING_FLOAT(Vehicle vehicle, char* class_, char* fieldName);
```

Returns the effective handling data of a vehicle as a floating-point value.
Example: `local fSteeringLock = GetVehicleHandlingFloat(vehicle, 'CHandlingData', 'fSteeringLock')`

## Parameters
* **vehicle**: The vehicle to obtain data for.
* **class_**: The handling class to get. Only "CHandlingData" is supported at this time.
* **fieldName**: The field name to get. These match the keys in `handling.meta`.

## Return value
A floating-point value.
