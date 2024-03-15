---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_XENON_LIGHTS_CUSTOM_COLOR

```c
BOOL GET_VEHICLE_XENON_LIGHTS_CUSTOM_COLOR(Vehicle vehicle, int* red, int* green, int* blue);
```

Returns vehicle xenon lights custom RGB color values. Do note this native doesn't return non-RGB colors that was set with [_SET_VEHICLE_XENON_LIGHTS_COLOR](#_0xE41033B25D003A07).

## Parameters
* **vehicle**: The vehicle handle.
* **red**: Red color (0-255).
* **green**: Green color (0-255).
* **blue**: Blue color (0-255).

## Return value
A boolean indicating if vehicle have custom xenon lights RGB color.
