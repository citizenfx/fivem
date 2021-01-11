---
ns: CFX
apiset: shared
---
## GET_RESOURCE_KVP_INT

```c
int GET_RESOURCE_KVP_INT(char* key);
```

A getter for [SET_RESOURCE_KVP_INT](#_0x6A2B1E8).

## Parameters
* **key**: The key to fetch

## Return value
A int that contains the value stored in the Kvp or nil/null if none.

## Examples

```lua
local kvpValue = GetResourceKvpInt('bananabread') 
if kvpValue then
	-- do something!
end
```
