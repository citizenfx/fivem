---
ns: CFX
apiset: client
---
## MUMBLE_IS_CONNECTED

```c
BOOL MUMBLE_IS_CONNECTED();
```

This native will return true if the user succesfully connected to the voice server.
If the user disabled the voice-chat setting it will return false.

## Return value
True if the player is connected to a mumble server.
