---
ns: CFX
apiset: shared
---
## GET_COLSHAPE_SPHERE_DATA

```c
BOOL GET_COLSHAPE_SPHERE_DATA(int colShapeId, float* x, float* y, float* z, float* radius);
```

Gets the data of a sphere collision shape.

## Parameters
* **colShapeId**: The collision shape ID.
* **x**: Center X coordinate output.
* **y**: Center Y coordinate output.
* **z**: Center Z coordinate output.
* **radius**: Sphere radius output.


## Return value
Returns true if the data was retrieved successfully. Returns false if the ID is invalid or the shape is of a different type.
