---
ns: CFX
apiset: client
---
## GET_DUI_HANDLE

```c
char* GET_DUI_HANDLE(long duiObject);
```

Returns the NUI window handle for a specified DUI browser object.

## Parameters
* **duiObject**: The DUI browser handle created with [CREATE_DUI](#_0x23EAF899).

## Return value
The NUI window handle, for use in e.g. [CREATE_RUNTIME_TEXTURE_FROM_DUI_HANDLE](#_0xB135472B).
