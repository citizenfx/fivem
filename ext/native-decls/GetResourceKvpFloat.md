---
ns: CFX
apiset: shared
---
## GET_RESOURCE_KVP_FLOAT

```c
float GET_RESOURCE_KVP_FLOAT(char* key);
```

A getter for [SET_RESOURCE_KVP_FLOAT](#_0x9ADD2938).

## Parameters
* **key**: The key to fetch

## Return value
A float that contains the value stored in the Kvp or nil/null if none.

## Examples

```lua
local kvpValue = GetResourceKvpInt('mollis') 
if kvpValue then
	-- do something!
end
```