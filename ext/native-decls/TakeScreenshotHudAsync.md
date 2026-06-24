---
ns: CFX
apiset: client
game: gta5
---
## TAKE_SCREENSHOT_HUD_ASYNC

```c
int TAKE_SCREENSHOT_HUD_ASYNC(int format, int quality);
```

Captures a screenshot of the next rendered frame including the HUD, and returns a handle that can be polled with [IS_SCREENSHOT_READY](#_0x3FAA3BE8) and retrieved with [GET_SCREENSHOT_ENCODED](#_0x3FAA3BE9).

Returns `-1` if too many screenshots are already in flight.

For a capture without the HUD, see [TAKE_SCREENSHOT_ASYNC](#_0x3FAA3BE6).


## Parameters
* **format**: `0` for JPEG, `1` for PNG.
* **quality**: JPEG quality from 1 to 100. Only used for JPEG.

## Return value
A handle for the pending screenshot, or `-1` on failure.

## Examples

```lua
local handle = TakeScreenshotHudAsync(0, 85)

CreateThread(function()
    while not IsScreenshotReady(handle) do
        Wait(0)
    end
    local data = GetScreenshotEncoded(handle)
    print('Screenshot with HUD captured: ' .. #data .. ' bytes')
end)
```

```js
const handle = TakeScreenshotHudAsync(0, 85);
const tick = setTick(() => {
    if (IsScreenshotReady(handle)) {
        const data = GetScreenshotEncoded(handle);
        console.log(`Screenshot with HUD captured: ${data.length} bytes`);
        clearTick(tick);
    }
});
```