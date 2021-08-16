---
ns: CFX
apiset: client
---
## MUMBLE_IS_CONNECTED

```c
NUMBER|BOOL MUMBLE_IS_CONNECTED();
```

This native will return 1 if the user succesfully connected to the voice server.
If the user disabled the voice-chat setting it will return false.

## Return value
1 if the player is connected to a mumble server.
