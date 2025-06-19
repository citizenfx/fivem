---
ns: CFX
apiset: client
game: gta5
---

## SET_LIGHT_INTERIOR

```c
bool SET_LIGHT_INTERIOR(int lightIndex, int interiorId, bool isPortal, int roomIndex);
```

Set the interior and room where the light should be active.

## Parameters

* **lightIndex**: The index of the created light
* **interiorId**: The ID of the interior where the light should be active
* **isPortal**: Attach to a portal or room
* **roomIndex**: The specific room

## Return value
A boolean indicating whether the operation succeeded.
