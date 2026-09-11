---
ns: CFX
apiset: client
game: rdr3
---
## GET_DOOR_HASH_FROM_ENTITY

```c
Hash GET_DOOR_HASH_FROM_ENTITY(Entity doorEntity);
```

Returns the Door System hash for a door entity handle. Reverse of `GET_ENTITY_BY_DOORHASH` (`0xF7424890E4A094C0`).

Looks up `CDoorsRendered` buckets and matches `DoorSystemEntry.ptrFwEntity == fwScriptGuid::GetBaseFromGuid(doorEntity)`. Returns `0` if the entity is not a Door System door.

Hash `0x76DA9001` (`joaat("GET_DOOR_HASH_FROM_ENTITY")` lowercase).

## Parameters

* **doorEntity**: Entity handle of a door (as returned by `GET_ENTITY_BY_DOORHASH` or `DOOR_SYSTEM_GET_ACTIVE`).

## Return value

Door hash, or `0` if not found.

## Example

```lua
local hash = GetDoorHashFromEntity(doorHandle)
if hash ~= 0 then
    DoorSystemSetDoorState(hash, 1) -- locked
end

-- round-trip check
local ent = GetEntityByDoorHash(hash)
assert(GetDoorHashFromEntity(ent) == hash)
```

## See also

* [GET_ENTITY_BY_DOORHASH](#_0xF7424890E4A094C0)
* [GET_CLOSEST_DOOR_HASH](#_0xF52EB2AA)
* [GET_CLOSEST_DOORS](#_0x2A1FD70C)
* [DOOR_SYSTEM_GET_ACTIVE](#_0xF65BBA4B)
