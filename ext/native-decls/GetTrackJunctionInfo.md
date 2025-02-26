---
ns: CFX
apiset: client
game: gta5
---
## GET_TRACK_JUNCTION_INFO

```c
bool GET_TRACK_JUNCTION_INFO(int junctionId, int* trackIndex, int* trackNode, int* newIndex, int* newNode, bool* direction);
```

## Parameters
* **junctionId**: The track junction handle
* **trackIndex**: The track index a train should be on
* **trackNode**: The node a train should be on
* **newIndex**: The new track index for a train to be placed on
* **newNode**: The new track node for a train to be placed on
* **direction**: The direction a train should be traveling for this junction

## Return value
Returns true if junction id is valid, false otherwise.