---
ns: CFX
apiset: client
---
## ADD_AUDIO_SUBMIX_OUTPUT

```c
void ADD_AUDIO_SUBMIX_OUTPUT(int submixId, int outputSubmixId);
```

Adds an output for the specified audio submix.

## Parameters
* **submixId**: The input submix.
* **outputSubmixId**: The output submix. Use `0` for the master game submix.
