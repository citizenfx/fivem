---
ns: CFX
apiset: client
---
## MUMBLE_CLEAR_VOICE_TARGET

```c
void MUMBLE_CLEAR_VOICE_TARGET(int targetId);
```

Clears the target list (typically set via [MUMBLE_ADD_VOICE_TARGET_CHANNEL](#_0x4D386C9E)) for the specified Mumble voice target ID.

## Parameters
* **targetId**: A Mumble voice target ID, ranging from 1..30 (inclusive).
