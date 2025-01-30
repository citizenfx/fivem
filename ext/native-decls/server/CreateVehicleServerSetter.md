---
ns: CFX
apiset: server
---
## CREATE_VEHICLE_SERVER_SETTER

```c
Vehicle CREATE_VEHICLE_SERVER_SETTER(Hash modelHash, char* type, float x, float y, float z, float heading);
```

Equivalent to CREATE_VEHICLE, but it uses 'server setter' logic (like the former CREATE_AUTOMOBILE) as a workaround for
reliability concerns regarding entity creation RPC.

Unlike CREATE_AUTOMOBILE, this supports other vehicle types as well.

## Parameters
* **modelHash**: The model of vehicle to spawn.
* **type**: The appropriate vehicle type for the model info. Can be one of `automobile`, `bike`, `boat`, `heli`, `plane`, `submarine`, `trailer`, and (potentially), `train`. This should be the same type as the `type` field in `vehicles.meta`.
* **x**: Spawn coordinate X component.
* **y**: Spawn coordinate Y component.
* **z**: Spawn coordinate Z component.
* **heading**: Heading to face towards, in degrees.

## Return value
A script handle for the vehicle, or 0 if the vehicle failed to be created.

## Examples
```lua
local heli = CreateVehicleServerSetter(`seasparrow`, 'heli', GetEntityCoords(GetPlayerPed(GetPlayers()[1])) + vector3(0, 0, 15), 0.0)
print(GetEntityCoords(heli)) -- should return correct coordinates
```