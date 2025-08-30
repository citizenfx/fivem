---
ns: CFX
apiset: client
game: rdr3
---
## REMAP_RAW_KEYMAP

```c
void REMAP_RAW_KEYMAP(char* keymapName, int newRawKeyIndex);
```

Remaps the keymap bound to `keymapName` to `newRawKeyIndex`

Virtual key codes can be found [here](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)

## Parameters
* **keymapName**: the name given to the keymap in [REGISTER_RAW_KEYMAP](#_0x49C1F6DC)
* **newRawKeyIndex**: Index of raw key from keyboard.


## Examples
```lua

function on_key_up()
	print("key no longer pressed")
end

function on_key_down()
	print("key is pressed")
end

local KEY_SPACE = 32
local canBeDisabled = false

local KEY_E = 69

RegisterRawKeymap("our_keymap", on_key_up, on_key_down, KEY_SPACE, canBeDisabled)

RemapRawKeymap("our_keymap", KEY_E)
```
