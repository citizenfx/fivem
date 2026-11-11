---
ns: CFX
apiset: client
game: gta5
---
## GET_CLOSEST_TRACK_NODES

```c
object GET_CLOSEST_TRACK_NODES(Vector3 position, float radius, bool includeDisabledTracks);
```

Get all track nodes and their track ids within the radius of the specified coordinates.

## Parameters
* **position**: Get track nodes at position
* **radius**: Get track nodes within radius
* **includeDisabledTracks**: Whether nodes on tracks disabled with [SET_TRACK_ENABLED](#_0x4B41E84C) should be included. Defaults to `false`, which only returns nodes on enabled tracks.

## Return value
Returns a list of tracks and node entries: a trackNode and a trackId

The data returned adheres to the following layout:
```
[{trackNode1, trackId1}, ..., {trackNodeN, trackIdN}]
```
