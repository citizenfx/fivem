---
ns: CFX
apiset: client
game: gta5
---
## CREATE_COLSHAPE_CYLINDER
```c
int CREATE_COLSHAPE_CYLINDER(float x, float y, float z, float radius, float height);
```
Creates a cylinder shape. The game will check for collision and fire either onPlayerEnterColshape or onPlayerLeaveColshape with the unique identifier.
## Parameters
* **x**: The X coordinate of the center of the cylinder.
* **y**: The Y coordinate of the center of the cylinder.
* **z**: The Z coordinate of the bottom of the cylinder.
* **radius**: The radius of the cylinder.
* **height**: The height of the cylinder.
## Return value
The unique identifier for the collision shape if successfully created, -1 if not.