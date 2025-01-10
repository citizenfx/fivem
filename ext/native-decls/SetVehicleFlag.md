---
ns: CFX
apiset: client
---
## SET_VEHICLE_FLAG

```c
bool SET_VEHICLE_FLAG(Vehicle vehicle, int flagIndex, bool value);
```

This native is a setter for [`GET_VEHICLE_HAS_FLAG`](#_0xD85C9F57).

## Parameters
* **vehicle**: The vehicle to set flag for.
* **flagIndex**: Flag index.
* **value**: `true` to enable the flag, `false` to disable it.


### Example
```lua
local vehicle = GetVehiclePedIsIn(PlayerPedId(), false)
if not vehicle then return end

SetVehicleFlag(vehicle, 43, true) -- Set the vehicle as electric 
```

```javascript
let vehicle = GetVehiclePedIsIn(PlayerPedId(), false);
if (!vehicle) return;

SetVehicleFlag(vehicle, 43, true);  // Set the vehicle as electric 
```