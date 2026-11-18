---
ns: CFX
apiset: shared
---
## GET_RESOURCE_KVP_STRING

```c
char* GET_RESOURCE_KVP_STRING(char* key, char* default);
```

A getter for [SET_RESOURCE_KVP](#_0x21C7A35B).

## Parameters
* **key**: The key to fetch
* **default**: The default value to set if none is found.

## Return value
The string value stored under the specified key, or nil/null/default if not found.

## Examples

```lua
local kvpValue = GetResourceKvpString('codfish') 
if kvpValue then
	-- do something!
end
```