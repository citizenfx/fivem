---
ns: CFX
apiset: shared
---
## GET_RESOURCE_BY_FIND_INDEX

```c
char* GET_RESOURCE_BY_FIND_INDEX(int findIndex);
```

## Parameters
* **findIndex**: The index of the resource (starting at 0)

## Return value
The resource name as a `string`

## Examples
```lua
local resourceList = {}
for i = 0, GetNumResources(), 1 do
  local resource_name = GetResourceByFindIndex(i)
  if resource_name and GetResourceState(resource_name) == "started" then
    table.insert(resourceList, resource_name)
  end
end
print(table.unpack(resourceList))
```
