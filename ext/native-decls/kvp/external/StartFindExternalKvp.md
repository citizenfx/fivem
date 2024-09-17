---
ns: CFX
apiset: client
---
## START_FIND_EXTERNAL_KVP

```c
int START_FIND_EXTERNAL_KVP(char* resourceName, char* prefix);
```

Equivalent of [START_FIND_KVP](#_0xDD379006), but for another resource than the current one.

## Parameters
* **resourceName**: The resource to try finding the key/values for
* **prefix**: A prefix match

## Return value
A KVP find handle to use with [FIND_KVP](#_0xBD7BEBC5) and close with [END_FIND_KVP](#_0xB3210203)

## Examples
```lua
local kvpHandle = StartFindExternalKvp('drugs', 'mollis:')

if kvpHandle ~= -1 then 
	local key
	
	repeat
		key = FindKvp(kvpHandle)

		if key then
			print(('%s: %s'):format(key, GetResourceKvpString(key)))
		end
	until not key

	EndFindKvp(kvpHandle)
else
	print('No KVPs found')
end
```
