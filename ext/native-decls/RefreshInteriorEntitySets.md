---
ns: CFX
apiset: client
game: gta5
---
## REFRESH_INTERIOR_ENTITY_SETS

```c
BOOL REFRESH_INTERIOR_ENTITY_SETS(int interiorId);
```

Applies the current entity set state of an interior (as set by `ACTIVATE_INTERIOR_ENTITY_SET`, `DEACTIVATE_INTERIOR_ENTITY_SET` and `SET_INTERIOR_ENTITY_SET_COLOR`) to the loaded interior, without reloading it.

`REFRESH_INTERIOR` depopulates the whole interior and rebuilds it over the next frames, so the entire interior disappears and reappears (and the scene drops out when the camera is inside it). This native only creates or deletes the entities of the entity sets that changed, leaving the rest of the interior untouched.

As with `REFRESH_INTERIOR`, the entities of a newly activated set show up once their models have streamed in.

Collisions that are part of the interior's static bounds (rather than of the entity set models themselves) are not updated.

## Examples

```lua
local interiorId = GetInteriorAtCoords(-1155.0, -1520.0, 10.0)

DeactivateInteriorEntitySet(interiorId, "set_a")
ActivateInteriorEntitySet(interiorId, "set_b")
RefreshInteriorEntitySets(interiorId)
```

## Parameters
* **interiorId**: The interior ID.

## Return value
`true` if the entity set state has been applied, `false` if the interior is invalid or isn't populated (the next population applies the entity set state).
