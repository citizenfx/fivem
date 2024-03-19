---
ns: CFX
apiset: client
#game: gta5 // commented out for compatibility reasons
---
## SET_MANUAL_SHUTDOWN_LOADING_SCREEN_NUI

```c
void SET_MANUAL_SHUTDOWN_LOADING_SCREEN_NUI(BOOL manualShutdown);
```

Sets whether or not `SHUTDOWN_LOADING_SCREEN` automatically shuts down the NUI frame for the loading screen. If this is enabled,
you will have to manually invoke `SHUTDOWN_LOADING_SCREEN_NUI` whenever you want to hide the NUI loading screen.

## Parameters
* **manualShutdown**: TRUE to manually shut down the loading screen NUI.

