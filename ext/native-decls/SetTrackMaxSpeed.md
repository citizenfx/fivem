---
ns: CFX
apiset: client
game: gta5
---
## SET_TRACK_MAX_SPEED

```c
void SET_TRACK_MAX_SPEED(int track, int newSpeed);
```

Sets the max speed for the train tracks. Used by ambient trains and for station calculations

## Parameters
* **track**: The track id (between 0 - 27)
* **newSpeed**: The tracks new speed 