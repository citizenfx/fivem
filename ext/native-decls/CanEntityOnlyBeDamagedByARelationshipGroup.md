---
ns: CFX
apiset: client
---
## CAN_ENTITY_ONLY_BE_DAMAGED_BY_A_RELATIONSHIP_GROUP

```c
bool CAN_ENTITY_ONLY_BE_DAMAGED_BY_A_RELATIONSHIP_GROUP(Entity entity, int* relationshipHash);
```

A getter for [SET_ENTITY_ONLY_DAMAGED_BY_RELATIONSHIP_GROUP](#_0x7022BD828FA0B082).

## Examples

```lua
local ped = PlayerPedId()

SetEntityOnlyDamagedByRelationshipGroup(ped, true, 1337) -- often used as 'stealthy' invincible  
local retval, relationshipHash = CanEntityOnlyBeDamagedByARelationshipGroup(ped)
print(retval, relationshipHash)-- retval: true, relationshipHash: 1337

SetEntityOnlyDamagedByRelationshipGroup(ped, false, 0)
local retval, relationshipHash = CanEntityOnlyBeDamagedByARelationshipGroup(ped)
print(retval, relationshipHash)-- retval: false, relationshipHash: 0
```

## Parameters
* **entity**: The entity to get the relationship damage status.
* **relationshipHash**: The relationshipHash out value.
