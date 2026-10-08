---
ns: CFX
apiset: shared
---
## GET_COLSHAPE_CUBOID_DATA

```c
BOOL GET_COLSHAPE_CUBOID_DATA(int colShapeId, float* x, float* y, float* z, float* width, float* depth, float* height);
```

Gets the data of a cuboid collision shape.

## Parameters
* **colShapeId**: The collision shape ID.
* **x**: Center X coordinate output.
* **y**: Center Y coordinate output.
* **z**: Center Z coordinate output.
* **width**: Width output.
* **depth**: Depth output.
* **height**: Height output.


## Return value
Returns true if the data was retrieved successfully. Returns false if the ID is invalid or the shape is of a different type.
