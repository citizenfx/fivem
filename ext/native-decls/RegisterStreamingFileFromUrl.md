---
ns: CFX
apiset: client
game: gta5
---
## REGISTER_STREAMING_FILE_FROM_URL

```c
void REGISTER_STREAMING_FILE_FROM_URL(char* registerAs, char* url);
```

**Experimental**: This native may be altered or removed in future versions of CitizenFX without warning.

Registers a file from an URL as a streaming asset in the GTA streaming subsystem. This will asynchronously register the asset, and caching is done based on the URL itself - cache headers are ignored.

Use `IS_STREAMING_FILE_READY` to check if the asset has been registered successfully.

## Parameters
* **registerAs**: The file name to register as, for example `asset.ydr`.
* **url**: The URL to fetch the asset from.