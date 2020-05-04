---
ns: CFX
apiset: client
---
## MUMBLE_SET_VOICE_TARGET

```c
void MUMBLE_SET_VOICE_TARGET(int targetId);
```

Sets the current Mumble voice target ID to broadcast voice to.

## Parameters
* **targetId**: A Mumble voice target ID, ranging from 1..30 (inclusive). 0 disables voice targets, and 31 is server loopback.
