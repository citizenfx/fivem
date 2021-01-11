---
ns: CFX
apiset: shared
---
## GET_RESOURCE_KVP_STRING

```c
char* GET_RESOURCE_KVP_STRING(char* key);
```

A getter for [SET_RESOURCE_KVP](#_0x21C7A35B).

## Parameters
* **key**: The key to fetch

## Return value
A string that contains the value stored in the Kvp or nil/null if none.

## Examples

```lua
local kvpValue = GetResourceKvpString('codfish') 
if kvpValue then
	-- do something!
end
```