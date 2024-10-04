---
ns: CFX
apiset: client
game: gta5
---
## SET_HANDLING_VECTOR

```c
void SET_HANDLING_VECTOR(char* vehicle, char* class_, char* fieldName, Vector3 value);
```

Sets a global handling override for a specific vehicle class. The name is supposed to match the `handlingName` field from handling.meta.
Example: `SetHandlingVector('AIRTUG', 'CHandlingData', 'vecCentreOfMassOffset', vector3(0.0, 0.0, -5.0))`

## Parameters
* **vehicle**: The vehicle class to set data for.
* **class_**: The handling class to set. Only "CHandlingData" is supported at this time.
* **fieldName**: The field name to set. These match the keys in `handling.meta`.
* **value**: The Vector3 value to set.

