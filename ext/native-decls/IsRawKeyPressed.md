---
ns: CFX
apiset: client
---
## IS_RAW_KEY_PRESSED

```c
BOOL IS_RAW_KEY_PRESSED(int rawKeyIndex);
```

Can be used to get state of raw key on keyboard.

Virtual key codes can be found [here](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)

## Parameters
* **rawKeyIndex**: Index of raw key from keyboard.

## Return value
Returns bool value of pressed state.

## Examples
```lua
if IsRawKeyPressed(32) then -- KEY_SPACE
    print("Spacebar pressed")
end
```
