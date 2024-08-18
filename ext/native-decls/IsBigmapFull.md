---
ns: CFX
apiset: client
game: gta5
---
## IS_BIGMAP_FULL

```c
BOOL IS_BIGMAP_FULL();
```

## Return value
Returns true if the full map is currently revealed on the minimap. 
Use [`IsBigmapActive`](#_0xFFF65C63) to check if the minimap is currently expanded or in it's normal state.

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
