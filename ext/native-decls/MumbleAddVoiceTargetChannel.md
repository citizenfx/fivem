---
ns: CFX
apiset: client
---
## MUMBLE_ADD_VOICE_TARGET_CHANNEL

```c
void MUMBLE_ADD_VOICE_TARGET_CHANNEL(int targetId, int channel);
```

Adds the specified channel to the target list for the specified Mumble voice target ID.

## Parameters
* **targetId**: A Mumble voice target ID, ranging from 1..30 (inclusive).
* **channel**: A game voice channel ID.
