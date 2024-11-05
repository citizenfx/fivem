---
ns: CFX
apiset: server
---
## GET_HOST_ID

```c
char* GET_HOST_ID();
```

This native is only used for non-onesync, it will return `null` for OneSync.

## Return value
Returns the player source of the current host, or `null` if there isn't onw.
