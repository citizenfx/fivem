---
ns: CFX
apiset: server
---
## GET_VEHICLE_TOTAL_REPAIRS

```c
int GET_VEHICLE_TOTAL_REPAIRS(Vehicle vehicle);
```


## Parameters
* **vehicle**: 

## Return value

Returns the total amount of repairs. Each repair will increase the count to make it possible to detect client repairs. 
The value has a range from 0 to 15. Next value after 15 is 0.
