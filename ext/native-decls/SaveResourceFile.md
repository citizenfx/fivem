---
ns: CFX
apiset: server
---
## SAVE_RESOURCE_FILE

```c
BOOL SAVE_RESOURCE_FILE(char* resourceName, char* fileName, char* data, int dataLength);
```

Writes the specified data to a file in the specified resource.
Using a length of `-1` will automatically detect the length assuming the data is a C string.

## Parameters
* **resourceName**: The name of the resource.
* **fileName**: The name of the file.
* **data**: The data to write to the file.
* **dataLength**: The length of the written data.

## Return value
A value indicating if the write succeeded.
