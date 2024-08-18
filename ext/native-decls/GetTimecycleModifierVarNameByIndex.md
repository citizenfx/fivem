---
ns: CFX
apiset: client
---
## GET_TIMECYCLE_MODIFIER_VAR_NAME_BY_INDEX

```c
char* GET_TIMECYCLE_MODIFIER_VAR_NAME_BY_INDEX(char* modifierName, int modifierVarIndex);
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
* **modifierName**: The name of timecycle modifier.
* **modifierVarIndex**: The index of a variable on the specified timecycle modifier.

## Return value
The name of a variable by index.
