---
ns: CFX
apiset: server
---
## GET_VEHICLE_CUSTOM_PRIMARY_COLOUR

```c
void GET_VEHICLE_CUSTOM_PRIMARY_COLOUR(Vehicle vehicle, int* r, int* g, int* b);
```


## Parameters
* **vehicle**: The vehicle to check
* **r**: 0-255 value.
* **g**: 0-255 value.
* **b**: 0-255 value.

## Return value
Return vehicle's RGB color based on primary color.

## Examples
```lua
local veh = GetVehiclePedIsIn(PlayerPedId(), false)
local r, g, b = GetVehicleCustomPrimaryColour(veh)
print(r, g, b)
```
