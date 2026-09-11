---
ns: CFX
apiset: client
game: rdr3
---
## GET_CLOSEST_DOOR_HASH

```c
Hash GET_CLOSEST_DOOR_HASH(float x, float y, float z, float maxDistance);
```

Returns the **door hash** (Door System id, not model hash) of the closest door to the given world position.

Iterates all `CDoorsRendered` buckets and measures squared distance via `fwEntity::GetPosition()` / `GET_ENTITY_COORDS`. Returns `0` if no door within `maxDistance`.

Hash `0xF52EB2AA` (`joaat("GET_CLOSEST_DOOR_HASH")` lowercase).

## Parameters

* **x**: X coordinate.
* **y**: Y coordinate.
* **z**: Z coordinate.
* **maxDistance**: Maximum search radius.

## Return value

Door hash of the closest door, or `0`.

## Example

```lua
local ped = PlayerPedId()
local coords = GetEntityCoords(ped)
local hash = GetClosestDoorHash(coords.x, coords.y, coords.z, 10.0)
if hash ~= 0 then
    print(("found door 0x%X state %d"):format(hash, DoorSystemGetDoorState(hash)))
end
```

## See also

* [DOOR_SYSTEM_GET_ACTIVE](#_0xF65BBA4B)
* [GET_CLOSEST_DOORS](#_0x2A1FD70C)
* [GET_DOOR_HASH_FROM_ENTITY](#_0x76DA9001)
* [GET_ENTITY_BY_DOORHASH](#_0xF7424890E4A094C0)
