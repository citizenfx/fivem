---
ns: CFX
apiset: client
---
## REGISTER_ENTITIES

```c
void REGISTER_ENTITIES(func factory);
```

**Experimental**: This native may be altered or removed in future versions of CitizenFX without warning.

Registers a set of entities with the game engine. These should match `CEntityDef` class information from the game.
At this time, this function **should not be used in a live environment**.

## Parameters
* **factory**: A function returning a list of entities.