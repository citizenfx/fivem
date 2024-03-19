---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_WHEEL_SURFACE_MATERIAL

```c
int GET_VEHICLE_WHEEL_SURFACE_MATERIAL(Vehicle vehicle, int wheelIndex);
```


## Parameters
* **vehicle**: The vehicle to obtain data for.
* **wheelIndex**: Index of wheel, 0-3.

## Return value
Integer representing the index of the current surface material of that wheel. Check materials.dat for the indexes.