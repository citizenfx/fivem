---
ns: CFX
apiset: client
---
## ADD_MINIMAP_OVERLAY

```c
int ADD_MINIMAP_OVERLAY(char* name);
```

Loads a minimap overlay from a GFx file in the current resource.

## Parameters
* **name**: The path to a `.gfx` file in the current resource. It has to be specified as a `file`.

## Return value
A minimap overlay ID.
