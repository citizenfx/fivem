---
ns: CFX
apiset: client
game: gta5
---
## RESTORE_VEHICLE_WHEEL

```c
void RESTORE_VEHICLE_WHEEL(Vehicle vehicle, int wheelIndex);
```

Restores a previously broken-off vehicle wheel by index. Inverse of [BREAK_OFF_VEHICLE_WHEEL](#_0xA274CADB).

Clears the broken-off state, restores tyre and suspension health to full, and reattaches the detached wheel fragment back to the vehicle.

Please note that this is not synchronized across the network.

## Examples
```lua
local vehicle = GetVehiclePedIsIn(PlayerPedId())

if DoesEntityExist(vehicle) then
  for i = 0, GetVehicleNumberOfWheels(vehicle) - 1 do
    if IsVehicleWheelBrokenOff(vehicle, i) then
      RestoreVehicleWheel(vehicle, i)
    end
  end
end
```

## Parameters
* **vehicle**: The vehicle handle.
* **wheelIndex**: The wheel index to restore.
