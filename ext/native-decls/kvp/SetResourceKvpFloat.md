---
ns: CFX
apiset: shared
---
## SET_RESOURCE_KVP_FLOAT

```c
void SET_RESOURCE_KVP_FLOAT(char* key, float value);
```

A setter for [GET_RESOURCE_KVP_FLOAT](#_0x35BDCEEA).

## Parameters
* **key**: The key to set
* **value**: The value to write

## Examples
```lua
local lickMy = 42.5
SetResourceKvpFloat('bananabread', lickMy)
```
