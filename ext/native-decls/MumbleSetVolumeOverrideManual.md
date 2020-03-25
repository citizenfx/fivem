---
ns: CFX
apiset: client
---
## MUMBLE_SET_VOLUME_OVERRIDE_MANUAL

```c
void MUMBLE_SET_VOLUME_OVERRIDE_MANUAL(int serverId, char* playerName, float volume);
```

Overrides the output volume for a particular player with the specified server id and player name on Mumble. This will also bypass 3D audio and distance calculations. -1.0 to reset the override.

## Parameters
* **serverId**: The player's server id.
* **playerName**: The player's name.
* **volume**: The volume, ranging from 0.0 to 1.0 (or above).

