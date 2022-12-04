---
ns: CFX
apiset: shared
---
## GET_CONVAR

```c
char* GET_CONVAR(char* varName, char* default_);
```

Can be used to get a console variable of type `char*`, for example a string.

## Examples
```lua
if GetConvar('voice_useNativeAudio', 'false') == 'true' then
    Citizen.Trace('Native Audio is enabled.')
end
```

## Parameters
* **varName**: The console variable to look up.
* **default_**: The default value to set if none is found.

## Return value
Returns the convar value if it can be found, otherwise it returns the assigned `default`.
