---
ns: CFX
apiset: client
game: gta5
---
## IS_SCREENSHOT_READY

```c
BOOL IS_SCREENSHOT_READY(int handle);
```

Gets whether the screenshot for the given handle has finished capturing and can be retrieved with [GET_SCREENSHOT_ENCODED](#_0x3FAA3BE9).

Handles are returned by [TAKE_SCREENSHOT_ASYNC](#_0x3FAA3BE6) or [TAKE_SCREENSHOT_HUD_ASYNC](#_0x3FAA3BE7).

This native returns false while the capture is pending and also if it failed. Failed captures are only detectable by calling [GET_SCREENSHOT_ENCODED](#_0x3FAA3BE9) and receiving an empty string, so poll with a timeout.

## Parameters
* **handle**: The screenshot handle returned by `TAKE_SCREENSHOT_ASYNC` or `TAKE_SCREENSHOT_HUD_ASYNC`.

## Return value
True when the screenshot is ready for retrieval.