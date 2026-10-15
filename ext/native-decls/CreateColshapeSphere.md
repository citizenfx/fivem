---
ns: CFX
apiset: shared
---
## CREATE_COLSHAPE_SPHERE

```c
int CREATE_COLSHAPE_SPHERE(float x, float y, float z, float radius);
```

Creates a spherical collision shape at the specified 3D position.

When an entity enters or leaves a collision shape, the `onColshapeEnter` and `onColshapeExit` events are triggered. Both events are called with the entity handle that triggered them and the collision shape ID.

## Parameters
* **x**: Center X coordinate.
* **y**: Center Y coordinate.
* **z**: Center Z coordinate.
* **radius**: Sphere radius.

## Return value
The collision shape ID, or -1 on failure.
