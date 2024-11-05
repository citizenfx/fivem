---
ns: CFX
apiset: client
---
## MUMBLE_REMOVE_VOICE_TARGET_CHANNEL

```c
void MUMBLE_REMOVE_VOICE_TARGET_CHANNEL(int targetId, int channel);
```

Removes the specified voice channel from the user's voice targets.

This is a remover for [MUMBLE_ADD_VOICE_TARGET_CHANNEL](#_0x4D386C9E)

## Parameters
* **targetId**: A Mumble voice target ID, ranging from 1..30 (inclusive).
* **channel**: The game voice channel ID to remove from the target.
