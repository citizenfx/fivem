---
ns: CFX
apiset: server
---
## GET_VEHICLE_COLOURS

```c
void GET_VEHICLE_COLOURS(Vehicle vehicle, int* colorPrimary, int* colorSecondary);
```

Gets the current vehicle color

Server variant of [GET_VEHICLE_COLOURS](#_0xA19435F193E081AC)

## Parameters
* **vehicle**: The vehicle to get the color of
* **colorPrimary**: The vehicles primary color
* **colorSecondary**: The vehicles secondary color

## Return value
For JS & Lua this will return `colorPrimary` and `colorSecondary`
