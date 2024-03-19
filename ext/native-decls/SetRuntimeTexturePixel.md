---
ns: CFX
apiset: client
game: gta5
---
## SET_RUNTIME_TEXTURE_PIXEL

```c
void SET_RUNTIME_TEXTURE_PIXEL(long tex, int x, int y, int r, int g, int b, int a);
```

Sets a pixel in the specified runtime texture. This will have to be committed using `COMMIT_RUNTIME_TEXTURE` to have any effect.

## Parameters
* **tex**: A handle to the runtime texture.
* **x**: The X position of the pixel to change.
* **y**: The Y position of the pixel to change.
* **r**: The new R value (0-255).
* **g**: The new G value (0-255).
* **b**: The new B value (0-255).
* **a**: The new A value (0-255).

