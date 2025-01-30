---
ns: CFX
apiset: client
game: gta5
---
## SET_TRACK_ENABLED

```c
void SET_TRACK_ENABLED(int track, bool enabled);
```

Toggles the track being active. If disabled mission trains will not be able to spawn on this track and will look for the next closest track to spawn

## Parameters
* **track**: The track id (between 0 - 27)
* **enabled**: Should this track be enabled