---
ns: CFX
apiset: server
---
## GET_VEHICLE_DOOR_LOCK_STATUS

```c
int GET_VEHICLE_DOOR_LOCK_STATUS(Vehicle vehicle);
```


## Parameters
* **vehicle**: The vehicle to check.

## Return value
```
None = 0,
Unlocked = 1,
Locked = 2,
LockedForPlayer = 3,
StickPlayerInside = 4, -- Doesn't allow players to exit the vehicle with the exit vehicle key.
CanBeBrokenInto = 7, -- Can be broken into the car. If the glass is broken, the value will be set to 1
CanBeBrokenIntoPersist = 8, -- Can be broken into persist
CannotBeTriedToEnter = 10, -- Cannot be tried to enter (Nothing happens when you press the vehicle enter key).
```

## Examples
```lua
local veh = GetVehiclePedIsIn(PlayerPedId(), false)
print(GetVehicleDoorLockStatus(veh))
```
