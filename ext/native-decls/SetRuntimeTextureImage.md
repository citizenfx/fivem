---
ns: CFX
apiset: client
game: gta5
---
## SET_RUNTIME_TEXTURE_IMAGE

```c
BOOL SET_RUNTIME_TEXTURE_IMAGE(long tex, char* fileName);
```

Replaces the pixel data in a runtime texture with the image data from a file in the current resource, or a data URL.

If the bitmap is a different size compared to the existing texture, it will be resampled.

This command may end up executed asynchronously, and only update the texture data at a later time.

## Parameters
* **tex**: A runtime texture handle.
* **fileName**: The file name of an image to load, or a base64 "data:" URL. This should preferably be a PNG, and has to be specified as a `file` in the resource manifest.

## Return value
TRUE for success, FALSE for failure.