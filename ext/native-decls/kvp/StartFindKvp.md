---
ns: CFX
apiset: shared
---
## START_FIND_KVP

```c
int START_FIND_KVP(char* prefix);
```


## Parameters
* **prefix**: A prefix match

## Return value
A KVP find handle to use with [FIND_KVP](#_0xBD7BEBC5) and close with [END_FIND_KVP](#_0xB3210203)

## Examples
```lua
SetResourceKvp('mollis:2', 'should be taken with alcohol')
SetResourceKvp('mollis:1', 'vesuvius citrate')
SetResourceKvp('mollis:manufacturer', 'Betta Pharmaceuticals')

local kvpHandle = StartFindKvp('mollis:')

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
