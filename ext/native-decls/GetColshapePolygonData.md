---
ns: CFX
apiset: shared
---
## GET_COLSHAPE_POLYGON_DATA

```c
BOOL GET_COLSHAPE_POLYGON_DATA(int colShapeId, float* minZ, float* maxZ);
```

Gets the vertical bounds of a polygon collision shape.

## Parameters
* **colShapeId**: The collision shape ID.
* **minZ**: Lower Z bound output.
* **maxZ**: Upper Z bound output.


## Return value
Returns true if the data was retrieved successfully. Returns false if the ID is invalid or the shape is of a different type.
