---
ns: CFX
apiset: client
---
## SEND_DUI_MESSAGE

```c
void SEND_DUI_MESSAGE(long duiObject, char* jsonString);
```

Sends a message to the specific DUI root page. This is similar to SEND\_NUI\_MESSAGE.

## Parameters
* **duiObject**: The DUI browser handle.
* **jsonString**: The message, encoded as JSON.

