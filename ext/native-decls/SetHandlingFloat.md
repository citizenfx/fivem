---
ns: CFX
apiset: client
game: gta5
---
## SET_HANDLING_FLOAT

```c
void SET_HANDLING_FLOAT(char* vehicle, char* class_, char* fieldName, float value);
```

Sets a global handling override for a specific vehicle class. The name is supposed to match the `handlingName` field from handling.meta.
Example: `SetHandlingFloat('AIRTUG', 'CHandlingData', 'fSteeringLock', 360.0)`

## Parameters
* **vehicle**: The vehicle class to set data for.
* **class_**: The handling class to set. Only "CHandlingData" is supported at this time.
* **fieldName**: The field name to set. These match the keys in `handling.meta`.
* **value**: The floating-point value to set.

