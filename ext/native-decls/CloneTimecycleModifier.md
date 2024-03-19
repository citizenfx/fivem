---
ns: CFX
apiset: client
---
## CLONE_TIMECYCLE_MODIFIER

```c
int CLONE_TIMECYCLE_MODIFIER(char* sourceModifierName, char* clonedModifierName);
```

## Examples

```lua
local sourceName = "underwater"
local cloneName = "my_awesome_timecycle"

local clonedIndex = CloneTimecycleModifier(sourceName, cloneName)
if clonedIndex ~= -1 then
  SetTimecycleModifier(cloneName)
end
```

## Parameters
* **sourceModifierName**: The source timecycle name.
* **clonedModifierName**: The clone timecycle name, must be unique.

## Return value
The cloned timecycle modifier index, or -1 if failed.
