---
ns: CFX
apiset: client
---
## SET_VEHICLE_WHEEL_TIRE_COLLIDER_SIZE

```c
void SET_VEHICLE_WHEEL_TIRE_COLLIDER_SIZE(Vehicle vehicle, int wheelIndex, float value);
```

Use along with SetVehicleWheelSize to resize the wheels (this native sets the collider size affecting physics while SetVehicleWheelSize will change visual size).

## Parameters
* **vehicle**: The vehicle to obtain data for.
* **wheelIndex**: Index of wheel, 0-3.
* **value**: Radius of tire collider.
