---
ns: CFX
apiset: client
game: gta5
---
## CREATE_RUNTIME_TXD

```c
long CREATE_RUNTIME_TXD(char* name);
```

Creates a runtime texture dictionary with the specified name.
Example:
```lua
local txd = CreateRuntimeTxd('meow')
```

## Parameters
* **name**: The name for the runtime TXD.

## Return value
A handle to the runtime TXD.
