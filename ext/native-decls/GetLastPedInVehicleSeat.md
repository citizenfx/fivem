---
ns: CFX
apiset: server
---

## GET_LAST_PED_IN_VEHICLE_SEAT

```c
Entity GET_LAST_PED_IN_VEHICLE_SEAT(Vehicle vehicle, int seatIndex);
```

## Parameters

- **vehicle**: The target vehicle.
- **seatIndex**: See eSeatPosition declared in [`IS_VEHICLE_SEAT_FREE`](#_0x22AC59A870E6A669).

## Return value

The last ped in the specified seat of the passed vehicle. Returns 0 if the specified seat was never occupied or the last ped does not exist anymore.
