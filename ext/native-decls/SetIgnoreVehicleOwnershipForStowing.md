---
ns: CFX
apiset: client
game: rdr3
---
## SET_IGNORE_VEHICLE_OWNERSHIP_FOR_STOWING

```c
void SET_IGNORE_VEHICLE_OWNERSHIP_FOR_STOWING(BOOL ignore);
```

Sets whether or not ownership checks should be performed while trying to stow a carriable on a hunting wagon.

## Parameters
* **ignore**: true to let the local player stow carriables on any hunting wagon, false to use the default behaviour.