---
ns: CFX
apiset: client
game: gta5
---
## DOES_VEHICLE_SUPPORT_DASHBOARD_COLOUR

```c
BOOL DOES_VEHICLE_SUPPORT_DASHBOARD_COLOUR(Vehicle vehicle);
```

Checks whether the materials of the vehicle take the dashboard colour (colour slot 7), so that [`_SET_VEHICLE_DASHBOARD_COLOR`](#_0x6089CDF6A57F326C) has a visible effect.

This reads the same per-model flags as [`GET_VEHICLE_COLOURS_WHICH_CAN_BE_SET`](#_0xEEBFC7A7EFDC35B4), which only covers colour slots 1 to 5. Like that native, it describes the model currently drawn: the high detail model once it is streamed in near the camera, the base model otherwise. A few vehicles differ between the two.

Vehicle mod parts are not taken into account: a vehicle can get a colourable dashboard from its mods (Benny's upgrades for instance) while this returns false.

## Examples
```lua
local vehicle = GetVehiclePedIsIn(PlayerPedId(), false)

if DoesVehicleSupportDashboardColour(vehicle) then
    SetVehicleDashboardColour(vehicle, 12)
end
```

## Parameters
* **vehicle**: The vehicle handle.

## Return value
True if the drawn model of the vehicle has materials taking the dashboard colour. False otherwise, or if the model of the vehicle is not drawable yet.
