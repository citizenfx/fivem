---
ns: CFX
apiset: shared
---
## GET_CONVAR_INT

```c
int GET_CONVAR_INT(char* varName, int defaultValue);
```

Can be used to get a console variable casted back to `int` (an integer value).

## Parameters
* **varName**: The console variable to look up.
* **defaultValue**: The default value to set if none is found (variable not set using [SET_CONVAR](#_0x341B16D2), or not accessible).

## Return value
Returns the convar value if it can be found, otherwise it returns the assigned `defaultValue`.

## Examples
```lua
if GetConvarInt('remainingRounds', 0) < 900 then
    Citizen.Trace("Less than 900 rounds remaining...")
end
```
