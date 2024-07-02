---
ns: CFX
apiset: client
game: gta5
---
## GET_GAME_POOL_SIZE

```c
int GET_GAME_POOL_SIZE(char* poolName, int* maxPoolSize);
```

### Supported pools
* `CPed`: Peds (including animals) and players.
* `CObject`: Objects (props), doors, and projectiles.
* `CVehicle`: Vehicles.
* `CPickup`: Pickups.

## Examples
```lua
local currentAmount, maxPoolSize = GetGamePoolSize('CObject') -- Get the current amount and max pool size of objects
print("CObject " .. currentAmount .. "/" .. maxPoolSize)
```

## Parameters
* **poolName**: The pool name to get the current amount and max size from.

## Return value
Returns the current amount of entries in the specified pool and the pools maximum size.