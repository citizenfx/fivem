---
ns: CFX
apiset: shared
---
## GET_RESOURCE_KVP_INT

```c
int GET_RESOURCE_KVP_INT(char* key);
```

This is resource specific, you can't get another resources Key-Value Pair.

## Parameters
* **key**: the key to fetch

## Return value
* The int related to that key, or null if it doesn't exist

## Example(s)

```lua
local showUi = GetResourceKvpInt('showUi') 
if showUi and showUi == 1 then
  -- show the Ui if its their first time!
end
```
