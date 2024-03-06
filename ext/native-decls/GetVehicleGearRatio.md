---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_GEAR_RATIO

```c
float GET_VEHICLE_GEAR_RATIO(Vehicle vehicle, int gear);
```

Gets vehicles gear ratio on choosen gear.

## Parameters
* **vehicle**: The vehicle handle.
* **gear**: The vehicles gear you want to get.

## Examples

```lua
local vehicle = GetVehiclePedIsIn(PlayerPedId(-1))
local currentGear = GetVehicleCurrentGear(Vehicle)

print(GetVehicleGearRatio(vehicle, currentGear)) -- will print current vehicle gear to console
```

```js
const vehicle = GetVehiclePedIsIn(PlayerPedId(-1));
const currentGear = GetVehicleCurrentGear(Vehicle);

console.log(GetVehicleGearRatio(vehicle, currentGear)); // will print current vehicle gear to console
```

```cs
using static CitizenFX.Core.API;

Vehicle vehicle = Game.PlayerPed.CurrentVehicle;
int currentGear = GetVehicleCurrentGear(vehicle.Handle);

Debug.WriteLine($"{GetVehicleGearRatio(vehicle.Handle, currentGear)}");
```