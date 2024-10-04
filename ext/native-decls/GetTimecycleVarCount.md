---
ns: CFX
apiset: client
---
## GET_TIMECYCLE_VAR_COUNT

```c
int GET_TIMECYCLE_VAR_COUNT();
```

Returns the amount of variables available to be applied on timecycle modifiers.

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

## Return value
The amount of available variables for timecycle modifiers.
