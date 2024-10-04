---
ns: CFX
apiset: client
game: gta5
---
## CREATE_RUNTIME_TEXTURE_FROM_IMAGE

```c
long CREATE_RUNTIME_TEXTURE_FROM_IMAGE(long txd, char* txn, char* fileName);
```

Creates a runtime texture from the specified file in the current resource or a base64 data URL.

## Parameters
* **txd**: A handle to the runtime TXD to create the runtime texture in.
* **txn**: The name for the texture in the runtime texture dictionary.
* **fileName**: The file name of an image to load or a base64 data URL. This should preferably be a PNG, and has to be specified as a `file` in the resource manifest.

## Return value
A runtime texture handle.
