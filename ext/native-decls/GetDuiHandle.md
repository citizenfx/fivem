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
* **duiObject**: The DUI browser handle.

## Return value
The NUI window handle, for use in e.g. CREATE\_RUNTIME\_TEXTURE\_FROM\_DUI\_HANDLE.
