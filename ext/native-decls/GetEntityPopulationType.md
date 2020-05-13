---
ns: CFX
apiset: server
---
## GET_ENTITY_POPULATION_TYPE

```c
int GET_ENTITY_POPULATION_TYPE(Entity entity);
```

This native gets an entity's population type.

## Parameters
* **entity**: the entity to obtain the population type from

## Return value

Returns the population type ID, defined by the below enumeration:

```cpp
enum ePopulationType
{
	POPTYPE_UNKNOWN = 0,
	POPTYPE_RANDOM_PERMANENT,
	POPTYPE_RANDOM_PARKED,
	POPTYPE_RANDOM_PATROL,
	POPTYPE_RANDOM_SCENARIO,
	POPTYPE_RANDOM_AMBIENT,
	POPTYPE_PERMANENT,
	POPTYPE_MISSION,
	POPTYPE_REPLAY,
	POPTYPE_CACHE,
	POPTYPE_TOOL
};
```
