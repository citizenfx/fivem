---
ns: CFX
apiset: client
game: gta5
---
## SET_TRACK_BRAKING_DISTANCE

```c
void SET_TRACK_BRAKING_DISTANCE(int track, float brakingDistance);
```

Sets the braking distance of the track. Used by trains to determine the point to slow down when entering a station.

## Parameters
* **track**: The track id (between 0 - 27)
* **brakingDistance**: The new braking distance