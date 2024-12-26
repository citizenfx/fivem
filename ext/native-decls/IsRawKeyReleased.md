---
ns: CFX
apiset: client
---
## IS_RAW_KEY_RELEASED

```c
BOOL IS_RAW_KEY_RELEASED(int rawKeyIndex);
```

Can be used to get release state of raw key on keyboard.

Virtual key codes can be found [here](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)

## Parameters
* **rawKeyIndex**: Index of raw key from keyboard.

## Return value
Returns bool value of released state.

## Examples
```lua
if IsRawKeyReleased(32) then -- KEY_SPACE
    print("Spacebar released")
end
```
