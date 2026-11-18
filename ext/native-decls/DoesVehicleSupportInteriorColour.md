---
ns: CFX
apiset: client
game: gta5
---
## DOES_VEHICLE_SUPPORT_INTERIOR_COLOUR

```c
BOOL DOES_VEHICLE_SUPPORT_INTERIOR_COLOUR(Vehicle vehicle);
```

Checks whether the materials of the vehicle take the interior colour (colour slot 6), so that [`_SET_VEHICLE_INTERIOR_COLOR`](#_0xF40DD601A65F7F19) has a visible effect.

This reads the same per-model flags as [`GET_VEHICLE_COLOURS_WHICH_CAN_BE_SET`](#_0xEEBFC7A7EFDC35B4), which only covers colour slots 1 to 5. Like that native, it describes the model currently drawn: the high detail model once it is streamed in near the camera, the base model otherwise. A few vehicles differ between the two.

Vehicle mod parts are not taken into account: a vehicle can get a colourable interior from its mods (Benny's upgrades for instance) while this returns false.

## Examples
```lua
local vehicle = GetVehiclePedIsIn(PlayerPedId(), false)

if DoesVehicleSupportInteriorColour(vehicle) then
    SetVehicleInteriorColour(vehicle, 12)
end
```

## Parameters
* **vehicle**: The vehicle handle.

## Return value
True if the drawn model of the vehicle has materials taking the interior colour. False otherwise, or if the model of the vehicle is not drawable yet.
