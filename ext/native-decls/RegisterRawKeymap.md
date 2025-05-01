---
ns: CFX
apiset: client
game: rdr3
---
## REGISTER_RAW_KEYMAP

```c
void REGISTER_RAW_KEYMAP(char* keymapName, func onKeyDown, func onKeyUp, int rawKeyIndex, BOOL canBeDisabled);
```

Registers a keymap that will be triggered whenever `rawKeyIndex` is pressed or released.

`onKeyUp` and `onKeyDown` will not provide any arguments.
```ts
function onStateChange();
```

## Parameters
* **keymapName**: A **unique** name that the keymap will be bound to, duplicates will result in the keymap not being registered.
* **onKeyDown**: The function to run when the key is being pressed, or `nil`.
* **onKeyUp**: The function to run when the key is no longer being pressed, or `nil`.
* **rawKeyIndex**: The virtual key to bind this keymap to, see a list [here](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)
* **canBeDisabled**: If calls to [DISABLE_RAW_KEY_THIS_FRAME](#_0x8BCF0014) will disable this keymap, if a keymap was disabled when the key was pressed down it will still call `onKeyUp` on release.

## Examples
```lua
function on_key_up()
	print("key no longer pressed")
end

function on_key_down()
	print("key is pressed")
end

local KEY_E = 69
local canBeDisabled = false


RegisterRawKeymap("our_keymap", on_key_up, on_key_down, KEY_E, canBeDisabled)
```
