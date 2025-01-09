---
ns: CFX
apiset: server
---
## GET_VEHICLE_DOOR_STATUS

```c
int GET_VEHICLE_DOOR_STATUS(Vehicle vehicle, cs_split int doorIndex);
```

Returns the open position of the specified door on the target vehicle.

## Parameters
- **vehicle**: The target vehicle.
- **doorIndex**: Index of door to check (0-6).

## Return value
A number from 0 to 7.