---
ns: CFX
apiset: client
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
local Vehicle = GetVehiclePedIsIn(PlayerPedId(-1))
local currentGear = GetVehicleCurrentGear(Vehicle)

print(GetVehicleGearRatio(Vehicle, currentGear)) -- will print current vehicle gear to console
```

```js
const Vehicle = GetVehiclePedIsIn(PlayerPedId(-1));
const currentGear = GetVehicleCurrentGear(Vehicle);

console.log(GetVehicleGearRatio(Vehicle, currentGear)); // will print current vehicle gear to console
```

```cs
using static CitizenFX.Core.API;

Vehicle veh = Game.PlayerPed.CurrentVehicle;
int currentGear = GetVehicleCurrentGear(veh.Handle);

Debug.WriteLine($"{GetVehicleGearRatio(veh.Handle, currentGear)}");
```