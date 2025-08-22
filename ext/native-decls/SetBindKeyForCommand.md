---
ns: CFX
apiset: client
game: gta5
---
## SET_BIND_KEY_FOR_COMMAND

```c
BOOL SET_BIND_KEY_FOR_COMMAND(char* command, char* source, char* key);
```

Set the key binding for the specified command.

## Parameters
* **command**: The command string to set the binding info for.
* **source**: The source of the key, e.g. "KEYBOARD", "MOUSE_BUTTON", etc.
* **key**: The key string to bind to the command.