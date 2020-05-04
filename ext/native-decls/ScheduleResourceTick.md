---
ns: CFX
apiset: server
---
## SCHEDULE_RESOURCE_TICK

```c
void SCHEDULE_RESOURCE_TICK(char* resourceName);
```

Schedules the specified resource to run a tick as soon as possible, bypassing the server's fixed tick rate.

## Parameters
* **resourceName**: The resource to tick.
