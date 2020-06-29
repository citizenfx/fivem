---
ns: CFX
apiset: client
---
## GET_GAME_POOL

```c
object GET_GAME_POOL(char* poolname);
```

Returns all pool handles for the given pool name; the data returned adheres to the following layout:
```
[ 770, 1026, 1282, 1538, 1794, 2050, 2306, 2562, 2818, 3074, 3330, 3586, 3842, 4098, 4354, 4610, ...]
```

### Supported Pools
**1**: CPed  
**2**: CObject  
**3**: CVehicle  
**4**: CPickup  

## Parameters
* **poolname**: 

## Return value
An object containing a list of all pool handles
