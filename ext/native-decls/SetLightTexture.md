---
ns: CFX
apiset: client
game: gta5
---
## SET_LIGHT_TEXTURE

```c
void SET_LIGHT_TEXTURE(char* textureDict, int textureHash);
```

Assign a texture to an existing light source, allowing custom light shapes or patterns using textures from streaming assets.

## Parameters

* **textureDict**: The name of the texture dictionary (TXD) containing the texture
* **textureHash**: Hash of the texture
