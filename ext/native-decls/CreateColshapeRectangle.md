---
ns: CFX
apiset: shared
---
## CREATE_COLSHAPE_RECTANGLE

```c
int CREATE_COLSHAPE_RECTANGLE(float x, float y, float z, float width, float depth, float heading);
```

Creates a rotated rectangle collision shape at the specified 3D position.

When an entity enters or leaves a collision shape, the `onColshapeEnter` and `onColshapeExit` events are triggered. Both events are called with the entity handle that triggered them and the collision shape ID.

## Parameters
* **x**: Center X coordinate.
* **y**: Center Y coordinate.
* **z**: Center Z coordinate.
* **width**: Total width.
* **depth**: Total depth.
* **heading**: Rotation heading in radians.

## Return value
The collision shape ID, or -1 on failure.
