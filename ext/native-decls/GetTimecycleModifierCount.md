---
ns: CFX
apiset: client
---
## GET_TIMECYCLE_MODIFIER_COUNT

```c
int GET_TIMECYCLE_MODIFIER_COUNT();
```

## Return value
Returns the amount of timecycle modifiers loaded.

## Examples

```lua
local count = GetTimecycleModifierCount()
print("we have  " .. count .. "timecycle modifiers loaded")
```
