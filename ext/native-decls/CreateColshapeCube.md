---
ns: CFX
apiset: client
game: gta5
---
## CREATE_COLSHAPE_CUBE

```c
int CREATE_COLSHAPE_CUBE(float x1, float y1, float z1, float x2, float y2, float z2);
```

Creates a cube shape. The game will check for collision and fire either onPlayerEnterColshape or onPlayerLeaveColshape with the unique identifier.

## Parameters
* **x1**: The X coordinate of the first corner of the cube.
* **y1**: The Y coordinate of the first corner of the cube.
* **z1**: The Z coordinate of the first corner of the cube.
* **x2**: The X coordinate of the opposite corner of the cube.
* **y2**: The Y coordinate of the opposite corner of the cube.
* **z2**: The Z coordinate of the opposite corner of the cube.

## Return value
The unique identifier for the collision shape if successfully created, -1 if not.