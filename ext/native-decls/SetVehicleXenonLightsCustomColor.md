---
ns: CFX
apiset: client
game: gta5
---
## SET_VEHICLE_XENON_LIGHTS_CUSTOM_COLOR

```c
void SET_VEHICLE_XENON_LIGHTS_CUSTOM_COLOR(Vehicle vehicle, int red, int green, int blue);
```

Sets custom vehicle xenon lights color, allowing to use RGB palette. The game will ignore lights color set by [_SET_VEHICLE_XENON_LIGHTS_COLOR](#_0xE41033B25D003A07) when custom color is active. This native is not synced between players. Requires xenon lights mod to be set on vehicle.

## Parameters
* **vehicle**: The vehicle handle.
* **red**: Red color (0-255).
* **green**: Green color (0-255).
* **blue**: Blue color (0-255).

## Examples
```lua
local vehicle = GetVehiclePedIsUsing(PlayerPedId())
if DoesEntityExist(vehicle) then
  -- Toggle xenon lights mod.
  ToggleVehicleMod(vehicle, 22, true)

  -- Set pink lights color.
  SetVehicleXenonLightsCustomColor(vehicle, 244, 5, 82)
end
```
