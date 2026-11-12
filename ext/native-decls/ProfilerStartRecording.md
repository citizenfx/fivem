---
ns: CFX
apiset: shared
---
## PROFILER_START_RECORDING

```c
void PROFILER_START_RECORDING(int frames, char* resourceName);
```

Starts recording on the profiler.

## Parameters
* **frames**: The amount of frames to record for, -1 to record until calling [PROFILER_STOP_RECORDING](#_0x2d29dea5)
* **resourceName**: The resource to record for, or null to record every resource.
