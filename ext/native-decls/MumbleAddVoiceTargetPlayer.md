---
ns: CFX
apiset: client
---
## MUMBLE_ADD_VOICE_TARGET_PLAYER

```c
void MUMBLE_ADD_VOICE_TARGET_PLAYER(int targetId, Player player);
```

Adds the specified player to the target list for the specified Mumble voice target ID.

Similar to [MUMBLE_ADD_VOICE_TARGET_CHANNEL](#_0x4D386C9E) except it specifically adds the player as a target instead of the voice channel

## Parameters
* **targetId**: A Mumble voice target ID, ranging from 1..30 (inclusive).
* **player**: A game player index.
