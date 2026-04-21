---
ns: CFX
apiset: client
---
## MUMBLE_ADD_VOICE_TARGET_PLAYER_BY_SERVER_ID

```c
void MUMBLE_ADD_VOICE_TARGET_PLAYER_BY_SERVER_ID(int targetId, int serverId);
```

Adds the specified player to the target list for the specified Mumble voice target ID.

Similar to [MUMBLE_ADD_VOICE_TARGET_CHANNEL](#_0x4D386C9E) except it specifically adds the player as a target instead of a voice channel

## Parameters
* **targetId**: A Mumble voice target ID, ranging from 1..30 (inclusive).
* **serverId**: The player's server id.
