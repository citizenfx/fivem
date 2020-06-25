---
ns: CFX
apiset: client
---
## SET_VEHICLE_WHEEL_TIRE_COLLIDER_WIDTH

```c
void SET_VEHICLE_WHEEL_TIRE_COLLIDER_WIDTH(Vehicle vehicle, int wheelIndex, float value);
```

Use along with SetVehicleWheelWidth to resize the wheels (this native sets the collider width affecting physics while SetVehicleWheelWidth will change visual width).

## Parameters
* **vehicle**: The vehicle to obtain data for.
* **wheelIndex**: Index of wheel, 0-3.
* **value**: Width of tire collider.
