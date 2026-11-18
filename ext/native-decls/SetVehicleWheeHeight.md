---
ns: CFX
apiset: client
game: gta5
---
## SET_VEHICLE_WHEEL_HEIGHT

```c
void SET_VEHICLE_WHEEL_HEIGHT(Vehicle vehicle, int wheelIndex, float height);
```

Adjusts the height of the specified wheel.  
Needs to be called every frame in order to function properly, as GTA will reset the offset otherwise.

## Parameters
* **vehicle**: The handle of the vehicle you want to edit.
* **wheelIndex**: The wheel index.
* **height**: The desired height for that wheel.
