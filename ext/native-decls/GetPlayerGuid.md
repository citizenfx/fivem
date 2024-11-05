---
ns: CFX
apiset: server
---
## GET_PLAYER_GUID

```c
char* GET_PLAYER_GUID(char* playerSrc);
```

This is a legacy native from an old authentication service, this now just provides a user generated GUID from the client based on the windows [GetTickTime](https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-gettickcount)

## Parameters
* **playerSrc**:

## Return value
Returns the GUID provided by the client, despite its name it is not a "global unique identifier" anymore.
