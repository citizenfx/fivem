---
ns: CFX
apiset: client
game: gta5
---
## SET_VEHICLE_SUSPENSION_HEIGHT

```c
void SET_VEHICLE_SUSPENSION_HEIGHT(Vehicle vehicle, float newHeight);
```

Sets the height of the vehicle's suspension.
This changes the same value set by Suspension in the mod shop.
Negatives values raise the car. Positive values lower the car.

This is change is visual only. The collision of the vehicle will not move.

## Parameters
* **vehicle**:
* **newHeight**:
