---
ns: CFX
apiset: shared
---
## SET_RESOURCE_KVP_INT

```c
void SET_RESOURCE_KVP_INT(char* key, int value);
```

A setter for [GET_RESOURCE_KVP_INT](#_0x557B586A).


## Parameters
* **key**: The key to set
* **value**: The value to write

## Examples
```lua
local lickMy = 42
SetResourceKvp('bananabread', lickMy)
```