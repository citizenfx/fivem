---
ns: CFX
apiset: shared
---
## IS_POINT_INSIDE_COLSHAPE

```c
BOOL IS_POINT_INSIDE_COLSHAPE(int colShapeId, float x, float y, float z);
```

Returns whether the given 3D point is inside the collision shape.

## Parameters
* **colShapeId**: The collision shape ID.
* **x**: Point X coordinate.
* **y**: Point Y coordinate.
* **z**: Point Z coordinate.

## Return value
Returns true if the point is inside the collision shape, false otherwise.
