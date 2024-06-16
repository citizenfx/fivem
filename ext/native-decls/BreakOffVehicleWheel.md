---
ns: CFX
apiset: client
game: gta5
---
## BREAK_OFF_VEHICLE_WHEEL

```c
void BREAK_OFF_VEHICLE_WHEEL(Vehicle vehicle, int wheelIndex, BOOL leaveDebrisTrail, BOOL deleteWheel, BOOL unknownFlag, BOOL putOnFire);
```

Break off vehicle wheel by index. The `leaveDebrisTrail` flag requires `putOnFire` to be true.

## Examples

```lua
local vehicle = GetVehiclePedIsIn(PlayerPedId())

if DoesEntityExist(vehicle) then
  for i = 0, 3 do
    BreakOffVehicleWheel(vehicle, i, true, false, true, false)
  end
end
```

## Parameters
* **vehicle**: The vehicle handle.
* **wheelIndex**: The wheel index.
* **leaveDebrisTrail**: Start "veh_debris_trail" ptfx.
* **deleteWheel**: True to delete wheel, otherwise detach.
* **unknownFlag**: Unknown flag.
* **putOnFire**: Set wheel on fire once detached.
