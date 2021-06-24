---
ns: CFX
apiset: server
---
## GET_PED_IN_VEHICLE_SEAT

```c
Entity GET_PED_IN_VEHICLE_SEAT(Vehicle vehicle, int index);
```

Seat indexes:
* 1 = Driver
* 2 = Front Right Passenger
* 3 = Back Left Passenger
* 4 = Back Right Passenger
* 5 = Further Back Left Passenger (vehicles > 4 seats)
* 6 = Further Back Right Passenger (vehicles > 4 seats)
* etc.

## Parameters
* **vehicle**: The target vehicle.
* **index**: The seat index.

## Return value
The ped in the specified seat of the passed vehicle. Returns 0 if the specified seat is not occupied.