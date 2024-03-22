---
ns: CFX
apiset: client
---
## LOAD_ROPE_DATA_FROM_PATH

```c
BOOL LOAD_ROPE_DATA_FROM_PATH(char* resourceName, char* fileName);
```

## Parameters
* **resourceName**: The name of the resource containing the file
* **fileName**: The name of the rope data file

Loads a rope data file from the specified resource, removing any previous rope data in the process.
Calling this native will remove all existing ropes in the world and unload rope textures.
For compatibility it is recommended to use common:/data/ropedata.xml as a template for the file and add additional entries on the end of it.

## Return value
True if the file was loaded otherwise false if the file does not exist.