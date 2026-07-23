---
ns: CFX
apiset: shared
---
## IS_COLSHAPE_POLYGON

```c
BOOL IS_COLSHAPE_POLYGON(int colShapeId);
```

Returns whether the collision shape with the given ID is a polygon.

## Parameters
* **colShapeId**: The collision shape ID.

## Return value
Returns true if the collision shape exists and is a polygon, false otherwise.
