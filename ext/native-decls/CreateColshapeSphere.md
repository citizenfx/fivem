---
ns: CFX
apiset: client
game: gta5
---
## CREATE_COLSHAPE_SPHERE

```c
int CREATE_COLSHAPE_SPHERE(float x, float y, float z, float radius);
```

Creates a sphere shape. The game will check for collision and fire either onPlayerEnterColshape or onPlayerLeaveColshape with the unique identifier.
## Parameters
* **x**: The X coordinate of the center of the sphere.
* **y**: The Y coordinate of the center of the sphere.
* **z**: The Z coordinate of the center of the sphere.
* **radius**: The radius of the sphere.

## Return value
The unique identifier for the collision shape if successfully created, -1 if not.