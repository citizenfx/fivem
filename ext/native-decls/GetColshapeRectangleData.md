---
ns: CFX
apiset: shared
---
## GET_COLSHAPE_RECTANGLE_DATA

```c
BOOL GET_COLSHAPE_RECTANGLE_DATA(int colShapeId, float* x, float* y, float* z, float* width, float* depth, float* heading);
```

Gets the data of a rectangle collision shape.

## Parameters
* **colShapeId**: The collision shape ID.
* **x**: Center X coordinate output.
* **y**: Center Y coordinate output.
* **z**: Center Z coordinate output.
* **width**: Width output.
* **depth**: Depth output.
* **heading**: Heading output in radians.


## Return value
Returns true if the data was retrieved successfully. Returns false if the ID is invalid or the shape is of a different type.
