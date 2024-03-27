---
ns: CFX
apiset: client
game: gta5
---
## SET_VEHICLE_GEAR_RATIO

```c
void SET_VEHICLE_GEAR_RATIO(Vehicle vehicle, int gear, float ratio);
```

Sets the vehicles gear ratio on choosen gear, reverse gear needs to be a negative float and forward moving gear needs to be a positive float. Refer to the examples if confused.

## Parameters
* **vehicle**: The vehicle handle.
* **gear**: The vehicles gear you want to change.
* **ratio**: The gear ratio you want to use.

## Examples

```lua
local function Set8SpeedVehicleGears(Vehicle)
    SetVehicleGearRatio(Vehicle, 0, -3.32)  -- reverse gear at -3.21:1
    SetVehicleGearRatio(Vehicle, 1, 4.71)   -- 1st gear at 4.71:1
    SetVehicleGearRatio(Vehicle, 2, 3.14)   -- 2nd gear at 3.14:1
    SetVehicleGearRatio(Vehicle, 3, 2.11)   -- 3rd gear at 2.11:1
    SetVehicleGearRatio(Vehicle, 4, 1.67)   -- 4th gear at 1.67:1
    SetVehicleGearRatio(Vehicle, 5, 1.29)   -- 5th gear at 1.29:1
    SetVehicleGearRatio(Vehicle, 6, 1.0)    -- 6th gear at 1.0:1
    SetVehicleGearRatio(Vehicle, 7, 0.84)   -- 7th gear at 0.84:1
    SetVehicleGearRatio(Vehicle, 8, 0.67)   -- 8th gear at 0.67:1
end
```

```js
function Set8SpeedVehicleGears(Vehicle) {
    SetVehicleGearRatio(Vehicle, 0, -3.32);  // reverse gear at -3.21:1
    SetVehicleGearRatio(Vehicle, 1, 4.71);   // 1st gear at 4.71:1
    SetVehicleGearRatio(Vehicle, 2, 3.14);   // 2nd gear at 3.14:1
    SetVehicleGearRatio(Vehicle, 3, 2.11);   // 3rd gear at 2.11:1
    SetVehicleGearRatio(Vehicle, 4, 1.67);   // 4th gear at 1.67:1
    SetVehicleGearRatio(Vehicle, 5, 1.29);   // 5th gear at 1.29:1
    SetVehicleGearRatio(Vehicle, 6, 1.0);    // 6th gear at 1.0:1
    SetVehicleGearRatio(Vehicle, 7, 0.84);   // 7th gear at 0.84:1
    SetVehicleGearRatio(Vehicle, 8, 0.67);   // 8th gear at 0.67:1
}
```

```cs
using static CitizenFX.Core.API;

public static void Set8SpeedVehicleGears(int Vehicle)
{
    SetVehicleGearRatio(Vehicle, 0, -3.32);  // reverse gear at -3.21:1
    SetVehicleGearRatio(Vehicle, 1, 4.71);   // 1st gear at 4.71:1
    SetVehicleGearRatio(Vehicle, 2, 3.14);   // 2nd gear at 3.14:1
    SetVehicleGearRatio(Vehicle, 3, 2.11);   // 3rd gear at 2.11:1
    SetVehicleGearRatio(Vehicle, 4, 1.67);   // 4th gear at 1.67:1
    SetVehicleGearRatio(Vehicle, 5, 1.29);   // 5th gear at 1.29:1
    SetVehicleGearRatio(Vehicle, 6, 1.0);    // 6th gear at 1.0:1
    SetVehicleGearRatio(Vehicle, 7, 0.84);   // 7th gear at 0.84:1
    SetVehicleGearRatio(Vehicle, 8, 0.67);   // 8th gear at 0.67:1
}
```