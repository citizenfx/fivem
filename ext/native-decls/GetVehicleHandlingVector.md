---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_HANDLING_VECTOR

```c
Vector3 GET_VEHICLE_HANDLING_VECTOR(Vehicle vehicle, char* class_, char* fieldName);
```

Returns the effective handling data of a vehicle as a vector value.
Example: `local inertiaMultiplier = GetVehicleHandlingVector(vehicle, 'CHandlingData', 'vecInertiaMultiplier')`

## Parameters
* **vehicle**: The vehicle to obtain data for.
* **class_**: The handling class to get. Only "CHandlingData" is supported at this time.
* **fieldName**: The field name to get. These match the keys in `handling.meta`.

## Return value
An integer.
