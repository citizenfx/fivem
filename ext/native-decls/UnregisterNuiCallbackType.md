---
ns: CFX
apiset: client
---
## UNREGISTER_NUI_CALLBACK_TYPE

```c
void UNREGISTER_NUI_CALLBACK_TYPE(char* type);
```

Will unregister and cleanup a NUI Callback type. 
Used within ScRT function definition to remove a NUI Callback.

## Parameters
* **type**: The type to unregister (types are registered with RegisterNuiCallbackType)