---
ns: CFX
apiset: server
---
## GET_VEHICLE_FLIGHT_NOZZLE_POSITION

```c
float GET_VEHICLE_FLIGHT_NOZZLE_POSITION(Vehicle vehicle);
```

Gets the flight nozzel position for the specified vehicle. See the client-side [_GET_VEHICLE_FLIGHT_NOZZLE_POSITION](#_0xDA62027C8BDB326E) native for usage examples.

## Parameters
* **vehicle**: The vehicle to check.

## Return value
The flight nozzel position between 0.0 (flying normally) and 1.0 (VTOL mode)