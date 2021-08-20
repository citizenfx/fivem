---
ns: CFX
apiset: server
---
## GET_VEHICLES_WITH_TYPE

```c
object GET_VEHICLES_WITH_TYPE(char* vehicleType);
```

### Vehicle types
- automobile
- bike
- boat
- heli
- plane
- submarine
- trailer
- train

The data returned adheres to the following layout:
```
[127, 42, 13, 37]
```

## Parameters
* **vehicleType**: 

## Return value
An object containing a list of vehicle handles.
