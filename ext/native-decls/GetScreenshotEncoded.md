---
ns: CFX
apiset: client
game: gta5
---
## GET_SCREENSHOT_ENCODED

```c
char* GET_SCREENSHOT_ENCODED(int handle);
```

Returns the base64-encoded image data for a screenshot captured with [TAKE_SCREENSHOT_ASYNC](#_0x3FAA3BE6) or [TAKE_SCREENSHOT_HUD_ASYNC](#_0x3FAA3BE7). The encoding matches the format chosen at capture time (JPEG or PNG).

The handle is freed after this call and cannot be reused.

Returns an empty string if the handle is invalid, not ready yet, or the capture failed. Use [IS_SCREENSHOT_READY](#_0x3FAA3BE8) to check readiness before calling.

## Parameters
* **handle**: The screenshot handle returned by `TAKE_SCREENSHOT_ASYNC` or `TAKE_SCREENSHOT_HUD_ASYNC`.

## Return value
The base64-encoded image data, or an empty string on failure.