---
ns: CFX
apiset: client
---
## MUMBLE_CLEAR_VOICE_TARGET_PLAYERS

```c
void MUMBLE_CLEAR_VOICE_TARGET_PLAYERS(int targetId);
```

Clears players from the target list for the specified Mumble voice target ID.

Unlike [MUMBLE_CLEAR_VOICE_TARGET](#_0x8555DCBA) this only resets channels, leaving targets added by [MUMBLE_ADD_VOICE_TARGET_CHANNEL](#_0x4D386C9E) in the voice targets

## Parameters
* **targetId**: A Mumble voice target ID, ranging from 1..30 (inclusive).
