---
ns: CFX
apiset: server
---
## GET_RESOURCE_PATH

```c
char* GET_RESOURCE_PATH(char* resourceName);
```

Returns the physical on-disk path of the specified resource.

## Parameters
* **resourceName**: The name of the resource.

## Return value
The resource directory name, possibly without trailing slash.
