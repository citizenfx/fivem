---
ns: CFX
apiset: shared
---
## GET_REGISTERED_COMMANDS

```c
object GET_REGISTERED_COMMANDS();
```

Returns all commands that are registered in the command system.
The data returned adheres to the following layout:
```
[
{
"name": "cmdlist"
},
{
"name": "command1"
}
]
```

## Return value
An object containing registered commands.
