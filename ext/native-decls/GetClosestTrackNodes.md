---
ns: CFX
apiset: client
game: gta5
---
## GET_CLOSEST_TRACK_NODES

```c
object GET_CLOSEST_TRACK_NODES(Vector3 position, float radius);
```

Get all track nodes and their track ids within the radius of the specified coordinates.

## Parameters
* **position**: Get track nodes at position
* **radius**: Get track nodes within radius

## Return value
Returns a list of tracks and node entries: a trackNode and a trackId

The data returned adheres to the following layout:
```
[{trackNode1, trackId1}, ..., {trackNodeN, trackIdN}]
```

## Return value
An object containing a list of track ids and track nodes that are within the specified radius.
