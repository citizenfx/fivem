---
ns: CFX
apiset: server
---
## GET_VEHICLE_NEON_ENABLED

```c
BOOL GET_VEHICLE_NEON_ENABLED(Vehicle vehicle, int neonIndex);
```

Getter to check if one of the neon lights of a vehicle is enabled. This native is the server side getter of [IS_VEHICLE_NEON_LIGHT_ENABLED](#_0x8C4B92553E4766A5).

```c
enum neonIndex
{
    NEON_BACK = 0,   // Back neon
    NEON_RIGHT = 1,  // Right neon
    NEON_LEFT = 2,   // Left neon
    NEON_FRONT = 3   // Front neon
};
```

## Parameters
* **vehicle**: The vehicle to check.
* **neonIndex**: A value from the neonIndex enum representing the neon light to check.

## Return value
Returns `true` if the specified neon light is enabled, `false` otherwise.

## Examples
```lua
local vehicle = GetVehiclePedIsIn(GetPlayerPed(1), false) -- 1 is the source here
local isRightNeonOn = GetVehicleNeonEnabled(vehicle, 1)
print(isRightNeonOn)
```

```js
const vehicle = GetVehiclePedIsIn(GetPlayerPed(1), false); // 1 is the source here
const isRightNeonOn = GetVehicleNeonEnabled(vehicle, 1);
console.log(isRightNeonOn);
```

```cs
using static CitizenFX.Core.Native.API;

int vehicle = GetVehiclePedIsIn(GetPlayerPed(1), false); // 1 is the source here
bool isRightNeonOn = GetVehicleNeonEnabled(vehicle, 1);
Debug.WriteLine($"{isRightNeonOn}");
```