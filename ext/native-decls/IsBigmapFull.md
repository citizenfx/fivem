---
ns: CFX
apiset: client
---
## IS_BIGMAP_FULL

```c
BOOL IS_BIGMAP_FULL();
```

<!-- Native implemented by Disquse. 0x66EE14B2 -->

Returns true if the full map is currently revealed on the minimap. 
Use [`IsBigmapActive`](#_0xFFF65C63) to check if the minimap is currently expanded or in it's normal state.


## Return value
Returns true if the full map is currently revealed on the minimap.

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
