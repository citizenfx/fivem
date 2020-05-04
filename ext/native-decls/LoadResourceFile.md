---
ns: CFX
apiset: shared
---
## LOAD_RESOURCE_FILE

```c
char* LOAD_RESOURCE_FILE(char* resourceName, char* fileName);
```

Reads the contents of a text file in a specified resource.
If executed on the client, this file has to be included in `files` in the resource manifest.
Example: `local data = LoadResourceFile("devtools", "data.json")`

## Parameters
* **resourceName**: The resource name.
* **fileName**: The file in the resource.

## Return value
The file contents
