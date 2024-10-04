---
ns: CFX
apiset: client
game: gta5
---
## SET_VEHICLE_WHEEL_IS_POWERED

```c
void SET_VEHICLE_WHEEL_IS_POWERED(Vehicle vehicle, int wheelIndex, BOOL powered);
```

Sets whether the wheel is powered.
On all wheel drive cars this works to change which wheels receive power, but if a car's fDriveBiasFront doesn't send power to that wheel, it won't get power anyway. This can be fixed by changing the fDriveBiasFront with SET_VEHICLE_HANDLING_FLOAT.
Max number of wheels can be retrieved with the native GET_VEHICLE_NUMBER_OF_WHEELS.
This is a shortcut to a flag in SET_VEHICLE_WHEEL_FLAGS.

## Parameters
* **vehicle**:
* **wheelIndex**:
* **powered**:

## Examples
```lua
SetVehicleWheelIsPowered(vehicle, 0, true);
```

```js
SetVehicleWheelIsPowered(vehicle, 0, true);
```

```cs
using static CitizenFX.Core.API;
// ...
// The elusive Left Wheel Drive
Vehicle veh = Game.PlayerPed.CurrentVehicle;
for(int i = 0; i < veh.Wheels.Count; i++){
    SetVehicleWheelIsPowered(veh.Handle, i, i % 2 == 0);
}
```
