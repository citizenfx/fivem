---
ns: CFX
apiset: client
---
## SET_VEHICLE_FLAG

```c
void SET_VEHICLE_FLAG(Vehicle vehicle, int flagIndex, bool value);
```

Set vehicle.meta flag by index. This native is a setter for [GET_VEHICLE_HAS_FLAG](#_0x7A2F9BA7)

**Note**: Depending on the game build, some vehicle flags may not exist as they were introduced in later versions.


## Parameters
* **vehicle**: The vehicle on which you want to set or unset a flag.
* **flagIndex**: The index of the flag to modify.
* **value**: A boolean to enable `true` or disable `false` the flag.

## Return value

## Examples
```lua
local vehicle = GetVehiclePedIsIn(PlayerPedId(), false)

SetVehicleFlag(vehicle, 43, true) -- Set the vehicle as a electric vehicle
```

```js
const vehicle = GetVehiclePedIsIn(PlayerPedId(), false);

SetVehicleFlag(vehicle, 43, true) // Set the vehicle as a electric vehicle
```