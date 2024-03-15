---
ns: CFX
apiset: client
game: gta5
---
## GET_ALL_ROPES

```c
object GET_ALL_ROPES();
```

Returns all rope handles. The data returned adheres to the following layout:
```
[ 770, 1026, 1282, 1538, 1794, 2050, 2306, 2562, 2818, 3074, 3330, 3586, 3842, 4098, 4354, 4610, ...]
```

## Return value
An object containing a list of all rope handles.
