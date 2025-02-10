---
ns: CFX
apiset: client
---
## DISABLE_RAW_KEY_THIS_FRAME

```c
BOOL DISABLE_RAW_KEY_THIS_FRAME(int rawKeyIndex);
```

Disables the specified `rawKeyIndex`, making it not trigger the regular `IS_RAW_KEY_*` natives.

Virtual key codes can be found [here](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)

## Parameters
* **rawKeyIndex**: Index of raw key from keyboard.

## Return value
Returns bool value of down state.

## Examples
```lua
local KEY_SPACE = 32
DisableRawKeyThisFrame(KEY_SPACE)
-- This will not get triggered this frame
if IsRawKeyDown(KEY_SPACE) then
	print("unreachable :(")
end
-- this will get triggered
if IsDisabledRawKeyDown(KEY_SPACE) then
    print("Spacebar is down")
end
```
