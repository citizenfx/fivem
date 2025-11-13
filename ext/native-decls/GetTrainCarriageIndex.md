---
ns: CFX
apiset: server
---
## GET_TRAIN_CARRIAGE_INDEX

```c
int GET_TRAIN_CARRIAGE_INDEX(Vehicle train);
```

## Parameters
* **train**: The entity handle.

## Return value
Returns the carriage index of the current train, or `-1` if used on a regular vehicle.
