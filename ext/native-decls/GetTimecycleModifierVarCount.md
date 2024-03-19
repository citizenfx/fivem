---
ns: CFX
apiset: client
---
## GET_TIMECYCLE_MODIFIER_VAR_COUNT

```c
int GET_TIMECYCLE_MODIFIER_VAR_COUNT(char* modifierName);
```

## Examples

```lua
local varCount = GetTimecycleModifierVarCount("underwater")

if varCount ~= 0 then
  for index = 0, varCount - 1 do
    local varName = GetTimecycleModifierVarNameByIndex(index)

    print(string.format("[%d] %s", index, varName))
  end
end
```

## Parameters
* **modifierName**: The timecycle modifier name.

## Return value
The amount of variables used on a specified timecycle modifier.
