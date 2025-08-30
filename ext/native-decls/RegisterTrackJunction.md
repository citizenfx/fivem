---
ns: CFX
apiset: client
game: gta5
---
## REGISTER_TRACK_JUNCTION

```c
int REGISTER_TRACK_JUNCTION(int trackIndex, int trackNode, int newIndex, int newNode, bool direction);
```

Registers a track junction that when enabled will cause a train on the defined trackIndex, node and direction to change its current track index and begin traveling on the new node

## Parameters
* **trackIndex**: The track index a train should be on
* **trackNode**: The node a train should be on
* **newIndex**: The new track index for a train to be placed on
* **newNode**: The new track node for a train to be placed on
* **direction**: The direction a train should be traveling for this junction

## Return value
The track junction's handle or -1 if invalid.