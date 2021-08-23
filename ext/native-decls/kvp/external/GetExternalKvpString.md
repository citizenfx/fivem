---
ns: CFX
apiset: client
---
## GET_EXTERNAL_KVP_STRING

```c
char* GET_EXTERNAL_KVP_STRING(char* resource, char* key);
```

A getter for [SET_RESOURCE_KVP](#_0x21C7A35B), but for a specified resource.

## Parameters
* **resource**: The resource to fetch from.
* **key**: The key to fetch

## Return value
A string that contains the value stored in the Kvp or nil/null if none.

## Examples

```lua
local kvpValue = GetExternalKvpString('food', 'codfish') 
if kvpValue then
	-- do something!
end
```