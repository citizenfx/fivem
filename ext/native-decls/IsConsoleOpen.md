---
ns: CFX
apiset: client
---
## IS_CONSOLE_OPEN

```c
BOOL IS_CONSOLE_OPEN();
```

Returns whether the in-game F8 console is currently open.

## Return value

`true` if the F8 console is open, `false` otherwise.

## Examples

```lua
RegisterCommand("checkconsole", function()
    if IsConsoleOpen() then
        print("F8 is open")
    else
        print("F8 is closed")
    end
end)
```
