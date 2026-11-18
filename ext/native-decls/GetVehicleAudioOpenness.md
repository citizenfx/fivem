---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_AUDIO_OPENNESS

```c
float GET_VEHICLE_AUDIO_OPENNESS(Vehicle vehicle);
```

Gets how "open" a vehicle's cabin is. This accounts for opened or broken doors, lowered or smashed windows, convertible roofs and similar, and is what the game uses to decide how much outside sound reaches the vehicle's occupants.

## Examples
```lua
local vehicle = GetVehiclePedIsIn(PlayerPedId(), false)

if DoesEntityExist(vehicle) then
  print(("Vehicle openness: %.2f"):format(GetVehicleAudioOpenness(vehicle)))
end
```

## Parameters
* **vehicle**: The target vehicle.

## Return value
A value between `0.0` (fully closed) and `1.0` (fully open), or `-1.0` if the vehicle is invalid.
