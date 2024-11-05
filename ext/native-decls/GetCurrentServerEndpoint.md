---
ns: CFX
apiset: client
---
## GET_CURRENT_SERVER_ENDPOINT

```c
char* GET_CURRENT_SERVER_ENDPOINT();
```

## Return value
Returns the peer address of the remote game server (e.g. `127.0.0.1:30120`) that the user is currently connected to, or `null` if not available.
