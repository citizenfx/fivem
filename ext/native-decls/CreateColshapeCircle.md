---
ns: CFX
apiset: shared
---
## CREATE_COLSHAPE_CIRCLE

```c
int CREATE_COLSHAPE_CIRCLE(float x, float y, float radius);
```

Creates a 2D circular collision shape at the specified position.

## Parameters
* **x**: Center X coordinate.
* **y**: Center Y coordinate.
* **radius**: Circle radius.

## Return value
The collision shape ID, or -1 on failure.
