---
ns: CFX
apiset: client
---
## IS_GAME_WINDOW_FOCUSED

```c
bool IS_GAME_WINDOW_FOCUSED();
```
Returns the game window focus state.

## Examples

```lua
local isGameFocused = IsGameWindowFocused()
print(isGameFocused)
```

```js
const isGameFocused = IsGameWindowFocused();
console.log(isGameFocused);
```

## Return value
True if the game window is focused, false otherwise.
