---
ns: CFX
apiset: server
---

## GET_PED_IN_VEHICLE_SEAT

```c
Entity GET_PED_IN_VEHICLE_SEAT(Vehicle vehicle, int seatIndex);
```

## Parameters

- **vehicle**: The target vehicle.
- **seatIndex**: See eSeatPosition declared in [`IS_VEHICLE_SEAT_FREE`](#_0x22AC59A870E6A669).

## Return value

The ped in the specified seat of the passed vehicle. Returns 0 if the specified seat is not occupied.
