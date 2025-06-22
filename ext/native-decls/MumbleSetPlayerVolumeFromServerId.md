---
ns: CFX
apiset: client
game: gta5
---
## MUMBLE_SET_PLAYER_VOLUME_FROM_SERVER_ID

```c
void MUMBLE_SET_PLAYER_VOLUME_FROM_SERVER_ID(int serverId, float volume);
```

Sets the voice volume for a specific player. The volume ranges from 0.0 to 1.0, with a default of 1.0.

## Parameters
* **serverId**: The player's server id.
* **volume**: The volume to set.
