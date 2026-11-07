---
ns: CFX
apiset: shared
---
## GET_RESOURCE_KVP_INT

```c
int GET_RESOURCE_KVP_INT(char* key, int default); 
```

A getter for [SET_RESOURCE_KVP_INT](#_0x6A2B1E8).

## Parameters
* **key**: The key to fetch
* **default**: The default value to set if none is found.

## Return value
The integer value stored under the specified key, or 0/default if not found.

## Examples

```lua
local kvpValue = GetResourceKvpInt('bananabread') 
if kvpValue ~= 0 then
	-- do something!
end
```
