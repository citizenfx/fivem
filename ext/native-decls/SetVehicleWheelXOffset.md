---
ns: CFX
apiset: client
game: gta5
---
## SET_VEHICLE_WHEEL_X_OFFSET

```c
void SET_VEHICLE_WHEEL_X_OFFSET(Vehicle vehicle, int wheelIndex, float offset);
```

Adjusts the offset of the specified wheel relative to the wheel's axle center.
Needs to be called every frame in order to function properly, as GTA will reset the offset otherwise.
This function can be especially useful to set the track width of a vehicle, for example:
```
function SetVehicleFrontTrackWidth(vehicle, width)
SetVehicleWheelXOffset(vehicle, 0, -width/2)
SetVehicleWheelXOffset(vehicle, 1, width/2)
end
```

## Parameters
* **vehicle**: 
* **wheelIndex**: 
* **offset**: 

