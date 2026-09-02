---
ns: CFX
apiset: shared
---
## CREATE_COLSHAPE_CYLINDER

```c
int CREATE_COLSHAPE_CYLINDER(float x, float y, float z, float radius, float height);
```

Creates a vertical cylinder collision shape at the specified 3D position.

## Parameters
* **x**: Center X coordinate.
* **y**: Center Y coordinate.
* **z**: Center Z coordinate.
* **radius**: Cylinder radius.
* **height**: Total height.

## Return value
The collision shape ID, or -1 on failure.
