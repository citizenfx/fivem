---
ns: CFX
apiset: server
---
## GET_VEHICLE_NEON_COLOUR

```c
void GET_VEHICLE_NEON_COLOUR(Vehicle vehicle, int* red, int* green, int* blue);
```

Getter to check the neon colour of a vehicle. This native is the server side getter of [GET_VEHICLE_NEON_LIGHTS_COLOUR](#_0x7619EEE8C886757F).


## Parameters
* **vehicle**: The vehicle to check.
* **red**: Pointer to an integer where the red component of the neon color will be stored.
* **green**: Pointer to an integer where the green component of the neon color will be stored.
* **blue**: Pointer to an integer where the blue component of the neon color will be stored.

## Return value
None. The neon color values are retrieved and stored in the `red`, `green`, and `blue` pointers. Make sure to store the returned values in variables for further use.

## Examples
```lua
local vehicle = GetVehiclePedIsIn(GetPlayerPed(1), false) -- 1 is the source here
local red, green, blue = GetVehicleNeonColour(vehicle)
print(red, green, blue)
```

```js
const vehicle = GetVehiclePedIsIn(GetPlayerPed(1), false); // 1 is the source here
const [red, green, blue] = GetVehicleNeonColour(vehicle);
console.log(red, green, blue);
```

```cs
using static CitizenFX.Core.Native.API;

int vehicle = GetVehiclePedIsIn(GetPlayerPed(1), false); // 1 is the source here
int red, green, blue;
GetVehicleNeonColour(vehicle, out red, out green, out blue);
Debug.WriteLine($"{red}, {green}, {blue}");
```