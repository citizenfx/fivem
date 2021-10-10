---
ns: CFX
apiset: client
---
## MUMBLE_REMOVE_VOICE_TARGET_PLAYER

```c
void MUMBLE_REMOVE_VOICE_TARGET_PLAYER(int targetId, Player player);
```

Removes the specified player from the user's voice targets.

Performs the opposite operation of [MUMBLE_ADD_VOICE_TARGET_PLAYER](#_0x32C5355A)

## Parameters
* **targetId**: A Mumble voice target ID, ranging from 1..30 (inclusive).
* **player**: The player index to remove from the target.
