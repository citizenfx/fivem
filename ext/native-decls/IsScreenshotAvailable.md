---
ns: CFX
apiset: client
game: rdr3
---
## IS_SCREENSHOT_AVAILABLE

```c
BOOL IS_SCREENSHOT_AVAILABLE();
```

Returns whether the client can currently capture a screenshot.

Capture reads the composited desktop, so it is only performed while the game window itself owns the foreground. As a result this returns false while the game is minimized, or while the player has another window focused.

Note that this does not make the capture game-only: anything drawn on top of a focused game window - overlays, notifications, other applications - still appears in the result.

## Return value
True when a capture would succeed right now, false otherwise.

## Examples
```lua
if IsScreenshotAvailable() then
    RequestScreenshot(90, 1920, function(image)
        print(('captured %d bytes of base64'):format(#image))
    end)
end
```
