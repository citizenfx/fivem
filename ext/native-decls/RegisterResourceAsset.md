---
ns: CFX
apiset: server
---
## REGISTER_RESOURCE_ASSET

```c
char* REGISTER_RESOURCE_ASSET(char* resourceName, char* fileName);
```

**Experimental**: This native may be altered or removed in future versions of CitizenFX without warning.

Registers a cached resource asset with the resource system, similar to the automatic scanning of the `stream/` folder.

## Parameters
* **resourceName**: The resource to add the asset to.
* **fileName**: A file name in the resource.

## Return value
A cache string to pass to `REGISTER_STREAMING_FILE_FROM_CACHE` on the client.