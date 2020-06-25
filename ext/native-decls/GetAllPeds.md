---
ns: CFX
apiset: server
---
## GET_ALL_PEDS

```c
object GET_ALL_PEDS();
```

Returns all peds handles known to the server.
The data returned adheres to the following layout:
```
[127, 42, 13, 37]
```

## Return value
An object containing a list of peds handles.
