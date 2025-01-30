---
ns: CFX
apiset: client
game: gta5
---
## GET_TRACK_BRAKING_DISTANCE

```c
float GET_TRACK_BRAKING_DISTANCE(int track);
```

## Parameters
* **track**: The track id (between 0 - 27)

## Return Value
The braking distance of the track. Used by trains to determine the point to slow down at when entering a station.