---
ns: CFX
apiset: server
---
## GET_VEHICLE_BODY_HEALTH

```c
float GET_VEHICLE_BODY_HEALTH(Vehicle vehicle);
```

**NOTE**: This native will always return `1000.0` if the client doesn't set the health to a value greater than/less than `1000.0` on the client.


## Parameters
* **vehicle**: The vehicle to get the body health of

## Return value
Returns the vehicles current body health
