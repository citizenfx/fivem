---
ns: CFX
apiset: client
---
## GET_GAME_POOL

```c
object GET_GAME_POOL(char* poolName);
```

Returns a list of entity handles (script GUID) for all entities in the specified pool - the data returned is an array as
follows:

```json
[ 770, 1026, 1282, 1538, 1794, 2050, 2306, 2562, 2818, 3074, 3330, 3586, 3842, 4098, 4354, 4610, ...]
```

### Supported pools
* `CPed`: Peds (including animals) and players.
* `CObject`: Objects (props), doors, and projectiles.
* `CVehicle`: Vehicles.
* `CPickup`: Pickups.

## Parameters
* **poolName**: The pool name to get a list of entities from.

## Return value
An array containing entity handles for each entity in the named pool.
