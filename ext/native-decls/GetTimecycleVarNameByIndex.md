---
ns: CFX
apiset: client
---
## GET_TIMECYCLE_VAR_NAME_BY_INDEX

```c
char* GET_TIMECYCLE_VAR_NAME_BY_INDEX(int varIndex);
```

See [GET_TIMECYCLE_VAR_COUNT](#_0x838B34D8).

## Examples

```lua
local varCount = GetTimecycleVarCount()

if varCount ~= 0 then
  for index = 0, varCount - 1 do
    local varName = GetTimecycleVarNameByIndex(index)
    local varDefault = GetTimecycleVarDefaultValueByIndex(index)

    print(string.format("[%d] %s (%f)", index, varName, varDefault))
  end
end
```

## Parameters
* **varIndex**: The index of variable.

## Return value
The name of a timecycle variable.
