---
ns: CFX
apiset: client
game: gta5
---
## CREATE_RUNTIME_TEXTURE

```c
long CREATE_RUNTIME_TEXTURE(long txd, char* txn, int width, int height);
```

Creates a blank runtime texture.

## Parameters
* **txd**: A handle to the runtime TXD to create the runtime texture in.
* **txn**: The name for the texture in the runtime texture dictionary.
* **width**: The width of the new texture.
* **height**: The height of the new texture.

## Return value
A runtime texture handle.
