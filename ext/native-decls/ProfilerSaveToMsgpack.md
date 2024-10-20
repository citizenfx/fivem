---
ns: CFX
apiset: shared
---
## PROFILER_SAVE_TO_MSGPACK

```c
void PROFILER_SAVE_TO_MSGPACK(char* fileName);
```

Saves the current profile in message pack format

On the server `fileName` will be the absolute path of where to save the profiler record to, like `C:\FiveM_Profilers`
On the client `fileName` will be the path relative to the `citizen/profiler` folder, by default this is `%localappdata%/FiveM/FiveM.app/citizen/profiler`

## Parameters
* **fileName**: The file name to save to, this should include file extensions, please see notes above.
