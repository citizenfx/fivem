---
ns: CFX
apiset: client
---
## DOOR_SYSTEM_GET_ACTIVE

```c
object DOOR_SYSTEM_GET_ACTIVE();
```

Returns a list of door system entries: a door system hash (see [ADD_DOOR_TO_SYSTEM](#_0x6F8838D03D1DC226)) and its object handle.

The data returned adheres to the following layout:
```
[{doorHash1, doorHandle1}, ..., {doorHashN, doorHandleN}]
```

## Return value
An object containing a list of door system entries.
