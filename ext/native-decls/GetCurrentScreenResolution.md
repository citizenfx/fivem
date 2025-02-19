---
ns: CFX
apiset: client
game: rdr3
---
## GET_CURRENT_SCREEN_RESOLUTION

```c
BOOL GET_CURRENT_SCREEN_RESOLUTION(int* width, int* height);
```

Gets the current screen resolution.

```lua
local success, width, height = GetCurrentScreenResolution()
if success then 
    print(string.format("Current screen resolution: %dx%d", width, height))
end
```