---
ns: CFX
apiset: shared
---
## CREATE_COLSHAPE_CUBOID

```c
int CREATE_COLSHAPE_CUBOID(float x, float y, float z, float width, float depth, float height);
```

Creates an axis-aligned box collision shape at the specified 3D position.

## Parameters
* **x**: Center X coordinate.
* **y**: Center Y coordinate.
* **z**: Center Z coordinate.
* **width**: Total width.
* **depth**: Total depth.
* **height**: Total height.

## Return value
The collision shape ID, or -1 on failure.
