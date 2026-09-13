---
ns: CFX
apiset: client
game: rdr3
---
## REQUEST_SCREENSHOT

```c
void REQUEST_SCREENSHOT(int quality, int maxDimension, func callback);
```

Captures the game window and hands the result to `callback` as a JPEG data URL (`data:image/jpeg;base64,...`).

Capture, downscale, JPEG encode and base64 encode all run on a background thread; the callback is invoked on the script thread once they finish. Several captures may be in flight at once, and the callback is dropped if the requesting resource stops before it completes.

Capture is only performed while the game window owns the foreground - see [`IS_SCREENSHOT_AVAILABLE`](#_0xCC71C452). When it is not, the callback still runs, with an empty string.

## Parameters
* **quality**: JPEG quality between 1 and 100. Pass 0 to use the default of 90.
* **maxDimension**: Longest edge in pixels the result may have; the image is downscaled with bilinear filtering when it is larger. Pass 0 to keep the native resolution. Values are clamped to 16..16384.
* **callback**: Invoked with the data URL, or with an empty string when the capture or encode failed. It is not invoked at all when the native is called without a callback or from outside a resource.

## Examples
```lua
RegisterCommand('capture', function()
    RequestScreenshot(90, 1920, function(image)
        if image == '' then
            print('capture failed')
            return
        end

        SendNuiMessage(json.encode({ type = 'screenshot', data = image }))
    end)
end)
```
