---
ns: CFX
apiset: client
---
## MUMBLE_REMOVE_VOICE_TARGET_PLAYER_BY_SERVER_ID

```c
void MUMBLE_REMOVE_VOICE_TARGET_PLAYER_BY_SERVER_ID(int targetId, int serverId);
```

Removes the specified player from the user's voice targets.

Performs the opposite operation of [MUMBLE_ADD_VOICE_TARGET_PLAYER_BY_SERVER_ID](#_0x25F2B65F)

## Parameters
* **targetId**: A Mumble voice target ID, ranging from 1..30 (inclusive).
* **serverId**: The player's server id to remove from the target.
