---
ns: CFX
apiset: server
---
## GET_NET_TYPE_FROM_ENTITY

```c
int GET_NET_TYPE_FROM_ENTITY(Entity entity);
```

Gets the specific entity type (as an integer), which can be one of the following defined down below:

#### FiveM:
```c
enum eNetObjEntityType
{
    Automobile = 0,
    Bike = 1,
    Boat = 2,
    Door = 3,
    Heli = 4,
    Object = 5,
    Ped = 6,
    Pickup = 7,
    PickupPlacement = 8,
    Plane = 9,
    Submarine = 10,
    Player = 11,
    Trailer = 12,
    Train = 13
};
```

#### RedM:
```c
enum eNetObjEntityType
{
    Animal = 0,
    Automobile = 1,
    Bike = 2,
    Boat = 3,
    Door = 4,
    Heli = 5,
    Object = 6,
    Ped = 7,
    Pickup = 8,
    PickupPlacement = 9,
    Plane = 10,
    Submarine = 11,
    Player = 12,
    Trailer = 13,
    Train = 14,
    DraftVeh = 15,
    StatsTracker = 16,
    PropSet = 17,
    AnimScene = 18,
    GroupScenario = 19,
    Herd = 20,
    Horse = 21,
    WorldState = 22,
    WorldProjectile = 23,
    Incident = 24,
    Guardzone = 25,
    PedGroup = 26,
    CombatDirector = 27,
    PedSharedTargeting = 28,
    Persistent = 29
};
```

## Parameters
* **entity**: The entity to get the specific type of.

## Return value
The specific entity type returned as an integer value or -1 if the entity is invalid.
