---
ns: CFX
apiset: shared
---
## GET_VEHICLE_TYPE

```c
char* GET_VEHICLE_TYPE(Vehicle vehicle);
```

Returns the type of the passed vehicle.

For client scripts, reference the more detailed [GET_VEHICLE_TYPE_RAW](#_0xDE73BC10) native.

### Vehicle types
- automobile
- bike
- boat
- heli
- plane
- submarine
- trailer
- train

## Parameters
* **vehicle**: The vehicle's entity handle.

## Return value
If the entity is a vehicle, the vehicle type. If it is not a vehicle, the return value will be null.