---
ns: CFX
apiset: client
---
## SEND_DUI_MOUSE_DOWN

```c
void SEND_DUI_MOUSE_DOWN(long duiObject, char* button);
```

Injects a 'mouse down' event for a DUI object. Coordinates are expected to be set using SEND\_DUI\_MOUSE\_MOVE.

## Parameters
* **duiObject**: The DUI browser handle.
* **button**: Either `'left'`, `'middle'` or `'right'`.

