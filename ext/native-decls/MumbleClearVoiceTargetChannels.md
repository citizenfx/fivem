---
ns: CFX
apiset: client
---
## MUMBLE_CLEAR_VOICE_TARGET_CHANNELS

```c
void MUMBLE_CLEAR_VOICE_TARGET_CHANNELS(int targetId);
```

Clears channels from the target list for the specified Mumble voice target ID.

Unlike [MUMBLE_CLEAR_VOICE_TARGET](#_0x8555DCBA) this only resets channels, leaving targets added by [MUMBLE_ADD_VOICE_TARGET_PLAYER](#_0x32C5355A) in the voice targets

## Parameters
* **targetId**: A Mumble voice target ID, ranging from 1..30 (inclusive).
