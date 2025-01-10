---
ns: CFX
apiset: shared
---
## GET_RESOURCE_COMMANDS

```c
object GET_RESOURCE_COMMANDS(char* resource);
```

Returns all commands registered by the specified resource.
The data returned adheres to the following layout:
```
[
{
"name": "cmdlist",
"resource": "example_resource",
"arity" = -1,
},
{
"name": "command1"
"resource": "example_resource2",
"arity" = -1,
}
]
```

## Return value
An object containing registered commands.

## Example

```lua
RegisterCommand("getCommands", function(_, args)
    local commands = GetResourceCommands(args[1])
    print(("Resource ^5%s^7 Has ^5%s^7 Commands Registered"):format(args[1], #commands))

    local commandList = ""
    for i=1, #commands do
        commandlist = commandList + ("%s, "):format(commands[i].name)
    end

    print(commandList)
end)
```
