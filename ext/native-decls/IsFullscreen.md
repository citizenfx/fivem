---
ns: CFX
apiset: client
---
## IS_FULLSCREEN

```c
bool IS_FULLSCREEN();
```
This native checks if the client window is in fullscreen.

## Return value

Returns `true` if the game window is in fullscreen, `false` otherwise.

## Examples

```lua
  local isInFullscreen = IsFullscreen()
  print(isInFullscreen)
```
