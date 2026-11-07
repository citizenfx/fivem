---
ns: CFX
apiset: client
game: gta5
---
## SET_VEHICLE_CURB_BOOST_ENABLED

```c
void SET_VEHICLE_CURB_BOOST_ENABLED(Vehicle vehicle, BOOL enabled);
```

Controls the longitudinal tyre-slip smoothing that can make a vehicle gain speed from kerbs and similar bumps.

Unlike enabling the `CF_USE_DOWNFORCE_BIAS` advanced handling flag, disabling curb boost with this native does not enable the Open Wheel suspension raise, downforce bias, or drag behavior.

This is a client-local setting. It remains active until the vehicle's wheel objects are recreated, the entity is deleted, or this native is called with `enabled` set to `true`. Reapply it after an ownership migration or other event that recreates the vehicle locally.

## Parameters

* **vehicle**: The vehicle to configure.
* **enabled**: Set to `false` to prevent curb boosting, or `true` to restore vanilla behavior.

## Examples

```lua
SetVehicleCurbBoostEnabled(vehicle, false)
```
