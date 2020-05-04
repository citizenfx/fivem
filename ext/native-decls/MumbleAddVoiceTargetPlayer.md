---
ns: CFX
apiset: client
---
## MUMBLE_ADD_VOICE_TARGET_PLAYER

```c
void MUMBLE_ADD_VOICE_TARGET_PLAYER(int targetId, Player player);
```

Adds the specified player to the target list for the specified Mumble voice target ID.

## Parameters
* **targetId**: A Mumble voice target ID, ranging from 1..30 (inclusive).
* **player**: A game player index.
