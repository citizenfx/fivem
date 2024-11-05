---
ns: CFX
apiset: client
---
## UNREGISTER_RAW_NUI_CALLBACK

```c
void UNREGISTER_RAW_NUI_CALLBACK(char* eventName);
```

Will unregister and cleanup a registered NUI callback handler.

Use with [`REGISTER_NUI_CALLBACK`](#_0xC59B980C) or [`REGISTER_RAW_NUI_CALLBACK`](#_0xA8AE9C2F) .

## Parameters
* **eventName**: The callback type to target

