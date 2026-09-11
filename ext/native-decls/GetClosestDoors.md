---
ns: CFX
apiset: client
game: rdr3
---
## GET_CLOSEST_DOORS

```c
object GET_CLOSEST_DOORS(float x, float y, float z, float maxDistance, BOOL sortByDistance);
```

Returns all Door System doors within `maxDistance` of the given position, with distance.

Hash `0x2A1FD70C` (`joaat("GET_CLOSEST_DOORS")` lowercase).

## Parameters

* **x**: X coordinate.
* **y**: Y coordinate.
* **z**: Z coordinate.
* **maxDistance**: Maximum search radius.
* **sortByDistance**: If true, sort results by distance ascending.

## Return value

An object containing a list of entries, each as

```
{doorHash, doorHandle, distance}
```

ordered by distance if `sortByDistance` is true.

## Example

```lua
local c = GetEntityCoords(PlayerPedId())
local doors = GetClosestDoors(c.x, c.y, c.z, 20.0, true)
for i, d in ipairs(doors) do
    print(("%d: hash 0x%X handle %d dist %.2f"):format(i, d[1], d[2], d[3]))
end
```

## See also

* [DOOR_SYSTEM_GET_ACTIVE](#_0xF65BBA4B)
* [GET_CLOSEST_DOOR_HASH](#_0xF52EB2AA)
* [GET_DOOR_HASH_FROM_ENTITY](#_0x76DA9001)
