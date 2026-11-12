+---
ns: CFX
apiset: server
---

## GET_ALL_ENTITIES_IN_ROUTING_BUCKET

```c
object GET_ALL_ENTITIES_IN_ROUTING_BUCKET(int bucket, BOOL filterPlayers);
```

Gets all entities that are in this routing bucket.
The data returned adheres to the following layout:
```
[127, 42, 13, 37]
```

Routing buckets are also known as 'dimensions' or 'virtual worlds' in past echoes, however they are population-aware.

## Parameters

- **bucket**: The routing bucket ID to get the entities from.
- **filterPlayers**: If this is true, it won't return the peds of players

## Return value

An object containing a list of entity handles.

## Examples

```lua
local function GetVehiclesFromRoutingBucket(bucket)
    local entities = GetAllEntitiesInRoutingBucket(bucket, true)
    local vehicles = {}

    for i = 1, #entities do
        local entity = entities[i]
        if GetEntityType(entity) == 2 then
            vehicles[#vehicles+1] = entity
        end
    end

    return vehicles
end
```

```js
const GetVehiclesFromRoutingBucket = (bucket) => {
    const entities = GetAllEntitiesInRoutingBucket(bucket, true);
    const vehicles = [];

    for (let i = 0; i < entities.length; i++) {
        const entity = entities[i];
        if (GetEntityType(entity) === 2) {
            vehicles.push(entity);
        }
    }

    return vehicles;
}
```

```cs
using static CitizenFX.Core.Native.API;

private Vehicle[] GetVehiclesFromRoutingBucket(int bucket) {
    const object entities = GetAllEntitiesInRoutingBucket(bucket, true);
    const Vehicle[] vehicles = [];
    int i = 0;

    foreach (int entity in entities) {
        if (GetEntityType(entity) == 2)
        {
            vehicles[i] = new Vehicle(entity);
        }
        i++;
    }

    return vehicles;
}
```