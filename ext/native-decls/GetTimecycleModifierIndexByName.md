---
ns: CFX
apiset: client
---
## GET_TIMECYCLE_MODIFIER_INDEX_BY_NAME

```c
int GET_TIMECYCLE_MODIFIER_INDEX_BY_NAME(char* modifierName);
```

## Examples

```lua
local modifierIndex = GetTimecycleModifierIndexByName("underwater")
local currentIndex = GetTimecycleModifierIndex()

if currentIndex ~= -1 and currentIndex == modifierIndex then
  print("we're actually using 'underwater' timecycle!")
end
```

## Parameters
* **modifierName**: The timecycle modifier name.

## Return value
The timecycle modifier index.
