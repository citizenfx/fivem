---
ns: CFX
apiset: client
---
## REMOVE_TIMECYCLE_MODIFIER_VAR

```c
void REMOVE_TIMECYCLE_MODIFIER_VAR(char* modifierName, char* varName);
```

## Examples

```lua
local modifierName = "superDARK"
local varName = "postfx_noise"

if DoesTimecycleModifierHasVar(modifierName, varName) then
  local success, value1, value2 = GetTimecycleModifierVar(modifierName, varName)

  if success then
    print(string.format("[%s] removed var %s with values: %f %f", modifierName, varName, value1, value2))
    RemoveTimecycleModifierVar(modifierName, varName)
  end
else
    SetTimecycleModifierVar(modifierName, varName, 1.0, 1.0)
    print(string.format("[%s] created var %s", modifierName, varName))
end
```

## Parameters
* **modifierName**: The name of timecycle modifier.
* **varName**: The name of timecycle variable.
