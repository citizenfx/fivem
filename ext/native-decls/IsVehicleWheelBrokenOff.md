---
ns: CFX
apiset: client
---
## IS_VEHICLE_WHEEL_BROKEN_OFF

```c
BOOL IS_VEHICLE_WHEEL_BROKEN_OFF(Vehicle vehicle, int wheelIndex);
```

Getter for [BREAK_OFF_VEHICLE_WHEEL](?_0xA274CADB).

## Examples
```lua
local vehicle = GetVehiclePedIsIn(PlayerPedId())

if DoesEntityExist(vehicle) then
  local isWheelBroken = IsVehicleWheelBrokenOff(vehicle, 1)
  print("Is wheel 1 broken? ", isWheelBroken)
end
```

## Parameters
* **vehicle**: The vehicle handle.
* **wheelIndex**: The wheel index.

## Return value