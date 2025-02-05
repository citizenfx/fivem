---
ns: CFX
apiset: client
game: gta5
---
## CREATE_COLSHAPE_RECTANGLE

```c
int CREATE_COLSHAPE_RECTANGLE(float x1, float y1, float x2, float y2, float bottomZ, float height);
```

Creates a rectangle shape. The game will check for collision and fire either onPlayerEnterColshape or onPlayerLeaveColshape with the unique identifier.

## Parameters
* **x1**: The X coordinate of the first corner.
* **y1**: The Y coordinate of the first corner.
* **x2**: The X coordinate of the opposite corner.
* **y2**: The Y coordinate of the opposite corner.
* **bottomZ**: The bottom Z coordinate of the rectangle.
* **height**: The height of the rectangle.

## Return value
The unique identifier for the collision shape if successfully created, -1 if not.