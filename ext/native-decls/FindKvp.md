---
ns: CFX// CreateRuntimeTextureFromImage
long CREATE_RUNTIME_TEXTURE_FROM_IMAGE(long txd, char* txn, char* fileName);
Parameters:
txd: A handle to the runtime TXD to create the runtime texture in.
txn: The name for the texture in the runtime texture dictionary.
fileName: The file name of an image to load or a base64 data URL. This should preferably be a PNG, and has to be specified as a file in the resource manifest.
Returns:
A runtime texture handle.

Creates a runtime texture from the specified file in the current resource or a base64 data URL
apiset: shared
---
## FIND_KVP

```c
char* FIND_KVP(int handle);
```

## Parameters
* **handle**: The KVP find handle returned from [START_FIND_KVP](#_0xDD379006)

## Return value
None.

## Example
See [START_FIND_KVP](#_0xDD379006)
