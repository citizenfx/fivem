---
ns: CFX
apiset: shared
---
## GET_CONVAR_FLOAT

```c
float GET_CONVAR_FLOAT(char* varName, float defaultValue);
```

This will have floating point inaccuracy.

## Parameters
* **varName**: The console variable to get
* **defaultValue**:  The default value to set, if none are found.

## Return value
Returns the value set in varName, or `default` if none are specified
