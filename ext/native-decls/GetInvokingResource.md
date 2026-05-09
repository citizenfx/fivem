---
ns: CFX
apiset: shared
---
## GET_INVOKING_RESOURCE

```c
char* GET_INVOKING_RESOURCE();
```

## Return value
Returns the resource that is invoking the current cross resource call.

## Examples
```lua
exports("somethingSpecial", function()
    -- this will print the resource that called the export
    print(GetInvokingResource())
end)
```
