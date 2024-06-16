---
ns: CFX
apiset: client
---
## CREATE_TIMECYCLE_MODIFIER

```c
int CREATE_TIMECYCLE_MODIFIER(char* modifierName);
```

Create a clean timecycle modifier. See [`SET_TIMECYCLE_MODIFIER_VAR`](#_0x6E0A422B) to add variables.

## Examples

```lua
local modifierName = "my_awesome_timecycle"
local createdIndex = CreateTimecycleModifier(modifierName)

if createdIndex ~= -1 then
  SetTimecycleModifier(modifierName)
end
```

## Parameters
* **modifierName**: The new timecycle name, must be unique.

## Return value
The created timecycle modifier index, or -1 if failed.
