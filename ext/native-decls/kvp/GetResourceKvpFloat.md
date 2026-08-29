---
ns: CFX
apiset: shared
---
## GET_RESOURCE_KVP_FLOAT

```c
float GET_RESOURCE_KVP_FLOAT(char* key, float default);
```

A getter for [SET_RESOURCE_KVP_FLOAT](#_0x9ADD2938).

## Parameters
* **key**: The key to fetch
* **default**: The default value to set if none is found.

## Return value
The floating-point value stored under the specified key, or 0.0/default if not found.

## Examples

```lua
local kvpValue = GetResourceKvpFloat('mollis')
if kvpValue ~= 0.0 then
	-- do something!
end
```