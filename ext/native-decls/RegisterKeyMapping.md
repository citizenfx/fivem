---
ns: CFX
apiset: client
---
## REGISTER_KEY_MAPPING

```c
void REGISTER_KEY_MAPPING(char* commandString, char* description, char* defaultMapper, char* defaultParameter);
```

Registers a key mapping for the current resource.

See the related [cookbook post](https://cookbook.fivem.net/2020/01/06/using-the-new-console-key-bindings/) for more information.

## Parameters
* **commandString**: The command to execute, and the identifier of the binding.
* **description**: A description for in the settings menu.
* **defaultMapper**: The mapper ID to use for the default binding, e.g. `keyboard`.
* **defaultParameter**: The IO parameter ID to use for the default binding, e.g. `f3`.

## Examples

```lua
local handsUp = false
CreateThread(function()
    while true do
        Wait(0)
        if handsUp then
            TaskHandsUp(PlayerPedId(), 250, PlayerPedId(), -1, true)
        end
    end
end)
RegisterCommand('+handsup', function()
    handsUp = true
end, false)
RegisterCommand('-handsup', function()
    handsUp = false
end, false)
RegisterKeyMapping('+handsup', 'Hands Up', 'keyboard', 'i')
```
