---
ns: CFX
apiset: client
---
## GET_VEHICLE_HAS_FLAG

```c
bool GET_VEHICLE_HAS_FLAG(Vehicle vehicle, int flagIndex);
```

Get vehicle.meta flag by index. Useful examples include FLAG_LAW_ENFORCEMENT (31), FLAG_RICH_CAR (36), FLAG_IS_ELECTRIC (43), FLAG_IS_OFFROAD_VEHICLE (48).

Complete list of flags: https://gtamods.com/wiki/Vehicles.meta#flags
Compilation of all vehicles' metadata files (including their flags): https://forum.cfx.re/t/vehicle-meta-files-last-dlc/5142301

## Parameters
* **vehicle**: The vehicle to obtain data for.
* **flagIndex**: Flag index (0-203)

## Return value
A boolean for whether the flag is set.