---
ns: CFX
apiset: client
game: gta5
---
## REGISTER_KEY_MAPPING

```c
void REGISTER_KEY_MAPPING(char* commandString, char* description, char* defaultMapper, char* defaultParameter);
```

Registers a key mapping for the current resource.

See the related [cookbook post](https://cookbook.fivem.net/2020/01/06/using-the-new-console-key-bindings/) for more information.

Below you can find some examples on how to create these keybindings as well as the alternate keybinding syntax, which is preceded by `~!` to indicate that it's an alternate key.

## Parameters
* **commandString**: The command to execute, and the identifier of the binding.
* **description**: A description for in the settings menu.
* **defaultMapper**: The [mapper ID](https://docs.fivem.net/docs/game-references/input-mapper-parameter-ids/) to use for the default binding, e.g. `keyboard`.
* **defaultParameter**: The [IO parameter ID](https://docs.fivem.net/docs/game-references/input-mapper-parameter-ids/) to use for the default binding, e.g. `f3`.

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

-- Alternate keybinding syntax
RegisterKeyMapping('~!+handsup', 'Hands Up - Alternate Key', 'keyboard', 'o')
```
