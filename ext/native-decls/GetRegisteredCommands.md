---
ns: CFX
apiset: shared
---
## GET_REGISTERED_COMMANDS

```c
object GET_REGISTERED_COMMANDS();
```


The data returned adheres to the following layout:
```
[
    {
        "name": "cmdlist",
        "resource": "resource",
        "arity": -1,
    },
    {
        "name": "command1"
        "resource": "resource_2",
        "arity": -1,
    },
    {
        "name": "command1",
        "arity": -1,
    }
]
```

## Return value
Returns all commands that are registered in the command system.

## Example

```lua
RegisterCommand("showCommands", function()
    local commands = GetRegisteredCommands()
    print(("There is currently ^5%s^7 commands registered"):format(#commands))

    local commandList = ""
    for i=1, #commands do
        -- make a nicely formated command list
        commandlist = commandList + ("%s: %s (arguments: %s)\n"):format(commands[i].resource, commands[i].name, commands[i].arity)
    end

    print(commandList)
end)
