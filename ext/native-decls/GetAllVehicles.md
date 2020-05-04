---
ns: CFX
apiset: server
---
## GET_ALL_VEHICLES

```c
object GET_ALL_VEHICLES();
```

Returns all vehicle handles known to the server.
The data returned adheres to the following layout:
```
[127, 42, 13, 37]
```

## Return value
An object containing a list of vehicle handles.
