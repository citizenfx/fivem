---
ns: CFX
apiset: client
---
## GET_EXTERNAL_KVP_FLOAT

```c
float GET_EXTERNAL_KVP_FLOAT(char* resource, char* key);
```

A getter for [SET_RESOURCE_KVP_FLOAT](#_0x9ADD2938), but for a specified resource.

## Parameters
* **resource**: The resource to fetch from.
* **key**: The key to fetch

## Return value
A float that contains the value stored in the Kvp or nil/null if none.

## Examples

```lua
local kvpValue = GetExternalKvpFloat('drugs', 'mollis') 
if kvpValue then
	-- do something!
end
```