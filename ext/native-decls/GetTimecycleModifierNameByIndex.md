---
ns: CFX
apiset: client
---
## GET_TIMECYCLE_MODIFIER_NAME_BY_INDEX

```c
char* GET_TIMECYCLE_MODIFIER_NAME_BY_INDEX(int modifierIndex);
```

## Examples

```lua
local modifierIndex = GetTimecycleModifierIndex()

if modifierIndex ~= -1 then
  local modifierName = GetTimecycleModifierNameByIndex(modifierIndex)
  print("current timecycle name is " .. modifierName)
end
```

## Parameters
* **modifierIndex**: The timecycle modifier index.

## Return value
The timecycle modifier name.
