---
ns: CFX
apiset: client
game: gta5
---
## GET_AVAILABLE_SCREEN_RESOLUTIONS

```c
object GET_AVAILABLE_SCREEN_RESOLUTIONS();
```

Returns the screen resolutions the game offers for the monitor it is currently using. This is the same list as
the pause menu's resolution option.

## Return value
A list of tables containing `width` and `height`.

## Examples
```lua
for _, resolution in ipairs(GetAvailableScreenResolutions()) do
    print(('%dx%d'):format(resolution.width, resolution.height))
end
```
