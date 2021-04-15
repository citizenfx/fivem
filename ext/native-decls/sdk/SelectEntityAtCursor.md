---
ns: CFX
apiset: client
game: gta5
---
## SELECT_ENTITY_AT_CURSOR

```c
Entity SELECT_ENTITY_AT_CURSOR(int hitFlags, BOOL precise);
```

Gets the selected entity at the current mouse cursor position, and changes the current selection depth. This function supports SDK infrastructure and is not intended to be used directly from your code.

## Parameters
* **hitFlags**: A bit mask of entity types to match.
* **precise**: Whether to do a _precise_ test, i.e. of visual coordinates, too.

## Return value
An entity handle, or zero.