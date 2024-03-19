---
ns: CFX
apiset: client
game: gta5
---
## SET_VEHICLE_WHEELIE_STATE

```c
void SET_VEHICLE_WHEELIE_STATE(Vehicle vehicle, int state);
```

Example script: https://pastebin.com/J6XGbkCW

List of known states:
```
1: Not wheeling.
65: Vehicle is ready to do wheelie (burnouting).
129: Vehicle is doing wheelie.
```

## Examples

```lua
Citizen.CreateThread(function()
  while true do
    Wait(1)

    local ped = PlayerPedId()
    local veh = GetVehiclePedIsUsing(ped)

    if veh ~= 0 then
      -- is vehicle a musclecar
      if GetVehicleClass(veh) == 4 then
        -- is ped a driver
        if GetPedInVehicleSeat(veh, -1) == ped then
          -- don't let vehicle to do wheelie
          SetVehicleWheelieState(veh, 1)
        end
      end
    end
  end
end)
```

## Parameters
* **vehicle**: Vehicle
* **state**: Wheelie state
