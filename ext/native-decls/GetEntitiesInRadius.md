---
ns: CFX
apiset: shared
---
## GET_ENTITIES_IN_RADIUS

```c
object GET_ENTITIES_IN_RADIUS(float x, float y, float z, float radius, int entityType, BOOL sortByDistance, object models);
```

### Supported types
* [1] : Peds (including animals) and players.
* [2] : Vehicles.
* [3] : Objects (props), doors, and projectiles.


### Coordinates need to be send unpacked (x,y,z)
```lua

-- Define the allowed model hashes
local allowedModelHashes = { GetHashKey("p_crate03x"), GetHashKey("p_crate22x") }

-- Get the player's current coordinates
local playerCoords = GetEntityCoords(PlayerPedId())

-- Retrieve all entities of type Object (type 3) within a radius of 10.0 units
-- that match the allowed model hashes
-- and sort output entities by distance
local entities = GetEntitiesInRadius(playerCoords.x, playerCoords.y, playerCoords.z, 10.0, 3, true, allowedModelHashes)

-- Iterate through the list of entities and print their ids
for i = 1, #entities do
    local entity = entities[i]
    print(entity)
end

```

## Parameters
* **x**: The X coordinate.
* **y**: The Y coordinate.
* **z**: The Z coordinate.
* **radius**: Max distance from coordinate to entity
* **entityType**: Entity types see list below
* **sortByDistance**: Sort output entites by distance from nearest to farthest
* **models**: List of allowed models its also optional

## Return value
An array containing entity handles for each entity.
