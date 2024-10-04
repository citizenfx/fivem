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
The integer value stored under the specified key, or 0 if not found.

## Examples

```lua
local kvpValue = GetResourceKvpInt('bananabread') 
if kvpValue ~= 0 then
	-- do something!
end
```
