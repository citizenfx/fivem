---
ns: CFX
apiset: server
---
## GET_VEHICLE_DOOR_LOCK_STATUS

```c
int GET_VEHICLE_DOOR_LOCK_STATUS(Vehicle vehicle);
```

```c
enum eVehicleLockStatus {
    None = 0,
    Locked = 2,
    LockedForPlayer = 3,
    // Doesn't allow players to exit the vehicle with the exit vehicle key.
    StickPlayerInside = 4,
    // Can be broken into the car. If the glass is broken, the value will be set to 1
    CanBeBrokenInto = 7,
    // Can be broken into persist
    CanBeBrokenIntoPersist = 8,
    // Cannot be entered (Nothing happens when you press the vehicle enter key).
    CannotBeTriedToEnter = 10,
}
```

It should be [noted](https://forum.cfx.re/t/4863241) that while the [client-side command](#_0x25BC98A59C2EA962) and its
setter distinguish between states 0 (unset) and 1 (unlocked), the game will synchronize both as state 0, so the server-side
command will return only '0' if unlocked.

## Parameters
* **vehicle**: A vehicle handle.

## Return value
The door lock status for the specified vehicle.
