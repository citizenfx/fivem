---
ns: CFX
apiset: client
game: gta5
---
## SET_LIGHT_TEXTURE

```c
bool SET_LIGHT_TEXTURE(int lightIndex, char* textureDict, int textureHash);
```

Assign a texture to an existing light source, allowing custom light shapes or patterns using textures from streaming assets.

## Parameters

* **lightIndex**: The index of the created light
* **textureDict**: The name of the texture dictionary (TXD) containing the texture
* **textureHash**: Hash of the texture

## Return value
A boolean indicating whether the operation succeeded.
