---
ns: CFX
apiset: server
---
## SCAN_RESOURCE_ROOT

```c
void SCAN_RESOURCE_ROOT(char* rootPath, func callback);
```

Scans the resources in the specified resource root. This function is only available in the 'monitor mode' process and is
not available for user resources.

## Parameters
* **rootPath**: The resource directory to scan.
* **callback**: A callback that will receive an object with results.
