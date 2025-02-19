---
ns: CFX
apiset: client
game: gta5
---
## CREATE_COLSHAPE_CIRCLE

```c
int CREATE_COLSHAPE_CIRCLE(float x, float y, float z, float radius);
```

Creates a circular shape. The game will check for collision and fire either onPlayerEnterColshape or onPlayerLeaveColshape with the unique identifier.

## Parameters
* **x**: The X coordinate of the center of the circle.
* **y**: The Y coordinate of the center of the circle.
* **z**: The Z coordinate of the center of the circle.
* **radius**: The radius of the circle.

## Return value
The unique identifier for the collision shape if successfully created, -1 if not.