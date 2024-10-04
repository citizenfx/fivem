---
ns: CFX
apiset: client
game: gta5
---
## IS_BIGMAP_ACTIVE

```c
BOOL IS_BIGMAP_ACTIVE();
```

Returns true if the minimap is currently expanded. False if it's the normal minimap state.
Use [`IsBigmapFull`](#_0x66EE14B2) to check if the full map is currently revealed on the minimap.


## Return value
A bool indicating if the minimap is currently expanded or normal state.

## Examples
```lua
local expanded = IsBigmapActive()
local fullMap = IsBigmapFull()
print("The minimap is currently " .. (expanded and "expanded" or "normal size") .. " and the full map is currently " .. (fullMap and "revealed" or "not revealed") .. ".")
```
```cs
bool expanded = IsBigmapActive();
bool fullMap = IsBigmapFull();

Debug.WriteLine($"The minimap is currently {(expanded ? "expanded" : "normal size")} and the full map is currently {(fullMap ? "revealed" : "not revealed")}.");
```
