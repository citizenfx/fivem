---
ns: CFX
apiset: client
game: rdr3
---
## GET_TRACK_FROM_INDEX

```c
int GET_TRACK_FROM_INDEX(int index);
```

## Examples

```lua
local trackHash = GetTrackFromIndex(idx)
```

## Parameters
* **index**: Index of the desired track

## Return value
The name hash of the train track at the given index, or 0 if the index is invalid.