---
ns: CFX
apiset: server
game: rdr3
---
## GET_MOUNT

```c
Ped GET_MOUNT(Ped ped);
```

## Parameters
* **ped**: the ped id

## Return value
Returns the entity the `ped` is currently on, or `0` if they're not on a mount.
