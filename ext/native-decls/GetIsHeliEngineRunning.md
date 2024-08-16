---
ns: CFX
apiset: server
---
## GET_IS_HELI_ENGINE_RUNNING

```c
BOOL GET_IS_HELI_ENGINE_RUNNING(Vehicle heli);
```

## Parameters
* **heli**: The helicopter to check

## Return value
Returns `true` if the helicopter's engine is running, `false` if it is not.