---
ns: CFX
apiset: server
game: gta5
---
## DOES_TRAIN_FORCE_DOORS_OPEN

```c
bool DOES_TRAIN_FORCE_DOORS_OPEN(Vehicle train);
```

Returns whether this train forces its doors open while a player is inside it.

This reads the flag as it is synchronised for the given train. The client-side `SET_TRAINS_FORCE_DOORS_OPEN` sets this for all trains at once.

## Parameters
* **train**: The train handle

## Return value
Returns true if the train forces its doors open.
