---
ns: CFX
apiset: client
---
## GET_EXTERNAL_KVP_INT

```c
int GET_EXTERNAL_KVP_INT(char* resource, char* key);
```

A getter for [SET_RESOURCE_KVP_INT](#_0x6A2B1E8), but for a specified resource.

## Parameters
* **resource**: The resource to fetch from.
* **key**: The key to fetch

## Return value
A int that contains the value stored in the Kvp or nil/null if none.

## Examples

```lua
local kvpValue = GetExternalKvpInt('food', 'bananabread') 
if kvpValue then
	-- do something!
end
```
