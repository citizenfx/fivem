---
ns: CFX
apiset: client
game: gta5
---
## SET_HANDLING_INT

```c
void SET_HANDLING_INT(char* vehicle, char* class_, char* fieldName, int value);
```

Sets a global handling override for a specific vehicle class. The name is supposed to match the `handlingName` field from handling.meta.

## Parameters
* **vehicle**: The vehicle class to set data for.
* **class_**: The handling class to set. Only "CHandlingData" is supported at this time.
* **fieldName**: The field name to set. These match the keys in `handling.meta`.
* **value**: The integer value to set.

