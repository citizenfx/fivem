---
ns: CFX
apiset: shared
---
## GET_CONVAR_BOOL

```c
BOOL GET_CONVAR_BOOL(char* varName, BOOL defaultValue);
```

Can be used to get a console variable casted back to `bool`.

## Parameters
* **varName**: The console variable to look up.
* **defaultValue**: The default value to set if none is found.

## Return value
Returns the convar value if it can be found, otherwise it returns the assigned `default`.

## Examples
```lua
if GetConvarBool('dev_mode', false) then
    print("Dev Mode is eanbled, load dev mode menus")
end
```
