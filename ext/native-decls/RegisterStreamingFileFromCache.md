---
ns: CFX
apiset: client
game: gta5
---
## REGISTER_STREAMING_FILE_FROM_CACHE

```c
void REGISTER_STREAMING_FILE_FROM_CACHE(char* resourceName, char* fileName, char* cacheString);
```

**Experimental**: This native may be altered or removed in future versions of CitizenFX without warning.

Registers a dynamic streaming asset from the server with the GTA streaming module system.

## Parameters
* **resourceName**: The resource to add the asset to.
* **fileName**: A file name in the resource.
* **cacheString**: The string returned from `REGISTER_RESOURCE_ASSET` on the server.