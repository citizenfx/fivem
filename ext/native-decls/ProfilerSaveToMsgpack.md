---
ns: CFX
apiset: shared
---
## PROFILER_SAVE_TO_MSGPACK

```c
char* PROFILER_SAVE_TO_MSGPACK();
```

Saves the current profile in JSON format.

Profiler recordings are saved in the `profiler` directory.

For servers this will be in the same directory that your `resources` folder is in.
For clients this will be in the same directory that your `citizen` folder is in, by default it is `%localappdata%/FiveM/FiveM.app/`

## Returns
Returns the profiler file name, or `null` if it failed to save the profile.
