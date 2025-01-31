---
ns: CFX
apiset: client
game: gta5
---
## CREATE_COLSHAPE_SPHERE
```c
bool CREATE_COLSHAPE_SPHERE(char* colShapeId, float x, float y, float z, float radius);
```
Creates a sphere shape. The game will check for collision and fire either onPlayerEnterColshape or onPlayerLeaveColshape with the colShapeId.
## Parameters
* **colShapeId**: The Identifier of the colshape.
* **x**: The X coordinate of the center of the sphere.
* **y**: The Y coordinate of the center of the sphere.
* **z**: The Z coordinate of the center of the sphere.
* **radius**: The radius of the sphere.
## Return value
true if created successfully, false if identifier is already taken.