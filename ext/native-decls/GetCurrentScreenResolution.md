---
ns: CFX
apiset: client
game: rdr3
---
## GET_CURRENT_SCREEN_RESOLUTION

```c
void GET_CURRENT_SCREEN_RESOLUTION(int* width, int* height);
```

Gets the current screen resolution.

```lua
local  width, height = GetCurrentScreenResolution()
print(string.format("Current screen resolution: %dx%d", width, height))

```