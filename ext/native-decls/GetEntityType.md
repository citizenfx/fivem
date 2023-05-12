---
ns: CFX
apiset: server
---
## GET_ENTITY_TYPE

```c
int GET_ENTITY_TYPE(Entity entity);
```

Gets the entity type (as an integer), which can be one of the following defined down below:

**The following entities will return type `1`:**

- Ped
- Player
- Animal (Red Dead Redemption 2)
- Horse (Red Dead Redemption 2)

**The following entities will return type `2`:**

- Automobile
- Bike
- Boat
- Heli
- Plane
- Submarine
- Trailer
- Train
- DraftVeh (Red Dead Redemption 2)

**The following entities will return type `3`:**

- Object
- Door
- Pickup

Otherwise, a value of `0` will be returned.



## Parameters
* **entity**: The entity to get the type of.

## Return value
The entity type returned as an integer value.
