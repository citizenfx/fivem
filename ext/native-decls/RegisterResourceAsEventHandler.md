---
ns: CFX
apiset: shared
---
## REGISTER_RESOURCE_AS_EVENT_HANDLER

```c
void REGISTER_RESOURCE_AS_EVENT_HANDLER(char* eventName);
```

An internal function which allows the current resource's HLL script runtimes to receive state for the specified event.

## Parameters
* **eventName**: An event name, or "*" to disable HLL event filtering for this resource.

