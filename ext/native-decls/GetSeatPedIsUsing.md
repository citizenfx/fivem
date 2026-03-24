---
ns: CFX
apiset: server
---
## GET_SEAT_PED_IS_USING

```c
int GET_SEAT_PED_IS_USING(Ped ped);
```

## Parameters
* **ped**: the ped id

## Return Value
Returns the seat index for specified `ped`, if the ped is not sitting in a vehicle it will return -3.
