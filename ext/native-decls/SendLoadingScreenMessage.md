---
ns: CFX
apiset: client
---
## SEND_LOADING_SCREEN_MESSAGE

```c
BOOL SEND_LOADING_SCREEN_MESSAGE(char* jsonString);
```

Sends a message to the `loadingScreen` NUI frame, which contains the HTML page referenced in `loadscreen` resources.

## Parameters
* **jsonString**: The JSON-encoded message.

## Return value
A success value.
