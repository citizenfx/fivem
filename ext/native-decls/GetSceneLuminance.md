---
ns: CFX
apiset: client
game: gta5
---
## GET_SCENE_LUMINANCE

```c
object GET_SCENE_LUMINANCE(float x, float y, float width, float height);
```

Returns how bright the scene behind a detection area looks, from `0` (black) to `1` (white) or `nil` in Lua / `null` in JavaScript and C# when no recent measurement is available.

The value follows perceived brightness rather than the physical amount of light: `0.5` is a medium grey, a dim room is around `0.3` and a night sky is close to `0`. Thresholds such as "use dark text above `0.5`" therefore behave as you would expect. Technically, the luminance of the area is averaged in linear light and returned as CIE L* lightness, scaled to `0`–`1`.

The scene is measured after tone mapping and before HUD, Scaleform and NUI are drawn, so a panel drawn over the detection area does not affect its own measurement.

Measurement starts on the first call and stops about a second after the last one, so the first calls return `nil` for a few frames as it is therefore asynchronously processed. The result is an estimate from a coarse grid over the screen, not an exact average of every pixel.

## Parameters

* **x**: Left edge, from `0` (left of the screen) to `1` (right).
* **y**: Top edge, from `0` (top of the screen) to `1` (bottom).
* **width**: Detection area width in screen units; must be positive.
* **height**: Detection area height in screen units; must be positive.

These are top-left coordinates, not the center coordinates used by `DRAW_RECT`. Detection area partially outside the screen are clipped.

## Return value

A number, or `nil` when unavailable or the detection area is empty, outside the screen, or contains NaN. Infinite values are clipped like any other out-of-range value. Although declared `object` to allow `nil`, this returns a number, not a table.

## Examples

```lua
CreateThread(function()
    while true do
        local brightness = GetSceneLuminance(0.05, 0.05, 0.3, 0.1)
        local useDarkText = brightness ~= nil and brightness > 0.5
        Wait(0)
    end
end)
```
