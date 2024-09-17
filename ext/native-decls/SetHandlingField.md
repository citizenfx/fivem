---
ns: CFX
apiset: client
game: gta5
---
## SET_HANDLING_FIELD

```c
void SET_HANDLING_FIELD(char* vehicle, char* class_, char* fieldName, Any value);
```

Sets a global handling override for a specific vehicle class. The name is supposed to match the `handlingName` field from handling.meta.
Example: `SetHandlingField('AIRTUG', 'CHandlingData', 'fSteeringLock', 360.0)`

## Parameters
* **vehicle**: The vehicle class to set data for.
* **class_**: The handling class to set. Only "CHandlingData" is supported at this time.
* **fieldName**: The field name to set. These match the keys in `handling.meta`.
* **value**: The value to set.

