---
ns: CFX
apiset: client
game: gta5
---
## IS_STREAMING_FILE_READY

```c
BOOL IS_STREAMING_FILE_READY(char* registerAs);
```

**Experimental**: This native may be altered or removed in future versions of CitizenFX without warning.

Returns whether an asynchronous streaming file registration completed.

## Parameters
* **registerAs**: The file name to check, for example `asset.ydr`.

## Return value
Whether or not the streaming file has been registered.