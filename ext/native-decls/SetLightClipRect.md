---
ns: CFX
apiset: client
game: gta5
---
## SET_LIGHT_CLIP_RECT

```c
bool SET_LIGHT_CLIP_RECT(int lightIndex, int x, int y, int width, int height);
```

Set the clip rectangle for a created light.

## Parameters

* **lightIndex**: The index of the created light
* **x**: The x-coordinate of the clip rectangle
* **y**: The y-coordinate of the clip rectangle
* **width**: The width of the clip rectangle
* **height**: The height of the clip rectangle

## Return value
A boolean indicating whether the operation succeeded.
