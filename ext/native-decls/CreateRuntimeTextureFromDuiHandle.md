---
ns: CFX
apiset: client
game: gta5
---
## CREATE_RUNTIME_TEXTURE_FROM_DUI_HANDLE

```c
long CREATE_RUNTIME_TEXTURE_FROM_DUI_HANDLE(long txd, char* txn, char* duiHandle);
```

Creates a runtime texture from a DUI handle.

## Parameters
* **txd**: A handle to the runtime TXD to create the runtime texture in.
* **txn**: The name for the texture in the runtime texture dictionary.
* **duiHandle**: The DUI handle returned from GET\_DUI\_HANDLE.

## Return value
The runtime texture handle.
