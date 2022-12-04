---
ns: CFX
apiset: server
---
## SET_CONVAR_REPLICATED

```c
void SET_CONVAR_REPLICATED(char* varName, char* value);
```

Used to replicate a server variable onto clients.

## Examples
```lua
SetConvarReplicated('voice_useNativeAudio', 'true')
```

## Parameters
* **varName**: The console variable name.
* **value**: The value to set for the given console variable.

