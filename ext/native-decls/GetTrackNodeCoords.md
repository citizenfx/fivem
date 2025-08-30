---
ns: CFX
apiset: client
game: gta5
---
## GET_TRACK_NODE_COORDS

```c
bool GET_TRACK_NODE_COORDS(int trackIndex, uint32_t trackNode, Vector3* coords);
```

Gets the coordinates of a specific track node.

## Parameters
* **trackIndex**: The track index
* **trackNode**: The track node
* **coords**: The resulting track node coords

## Return value
Returns if it succeeds in getting coords or not