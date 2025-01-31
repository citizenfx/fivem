---
ns: CFX
apiset: client
game: gta5
---
## CREATE_COLSHAPE_CIRCLE
```c
bool CREATE_COLSHAPE_CIRCLE(char* colShapeId, float x, float y, float z, float radius);
```
Creates a circular shape. The game will check for collision and fire either onPlayerEnterColshape or onPlayerLeaveColshape with the colShapeId.
## Parameters
* **colShapeId**: The Identifier of the colshape.
* **x**: The X coordinate of the center of the circle.
* **y**: The Y coordinate of the center of the circle.
* **z**: The Z coordinate of the center of the circle.
* **radius**: The radius of the circle.
## Return value
true if created successfully, false if identifier is already taken.