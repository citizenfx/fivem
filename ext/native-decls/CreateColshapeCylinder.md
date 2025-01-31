---
ns: CFX
apiset: client
game: gta5
---
## CREATE_COLSHAPE_CYLINDER
```c
bool CREATE_COLSHAPE_CYLINDER(char* colShapeId, float x, float y, float z, float radius, float height);
```
Creates a cylinder shape. The game will check for collision and fire either onPlayerEnterColshape or onPlayerLeaveColshape with the colShapeId.
## Parameters
* **colShapeId**: The Identifier of the colshape.
* **x**: The X coordinate of the center of the cylinder.
* **y**: The Y coordinate of the center of the cylinder.
* **z**: The Z coordinate of the bottom of the cylinder.
* **radius**: The radius of the cylinder.
* **height**: The height of the cylinder.
## Return value
true if created successfully, false if identifier is already taken.