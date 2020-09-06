---
ns: CFX
apiset: server
---
## GET_ALL_OBJECTS

```c
object GET_ALL_OBJECTS();
```

Returns all object handles known to the server.
The data returned adheres to the following layout:
```
[127, 42, 13, 37]
```

## Return value
An object containing a list of object handles.
