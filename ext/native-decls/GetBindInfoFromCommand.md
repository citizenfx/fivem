---
ns: CFX
apiset: client
game: gta5
---
## GET_BIND_INFO_FROM_COMMAND

```c
object GET_BIND_INFO_FROM_COMMAND(char* command);
```

Return a object with the info of the bind, containing tag, description, keyName, sourceName, parameter, found and hasKey.

## Parameters
* **command**: The command string to get the binding info for.