---
ns: CFX
apiset: client
---
## GET_CURRENT_SERVER_ENDPOINT

```c
char* GET_CURRENT_SERVER_ENDPOINT();
```

Returns the peer address of the remote game server that the user is currently connected to.

## Return value
The peer address of the game server (e.g. `127.0.0.1:30120`), or NULL if not available.