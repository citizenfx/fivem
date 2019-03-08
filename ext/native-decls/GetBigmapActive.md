---
ns: CFX
apiset: client
---
## GET_BIGMAP_ACTIVE

```c
void GET_BIGMAP_ACTIVE(BOOL* expandedRadar, BOOL* revealFullMap);
```

<!-- Native implemented by Disquse. 0xF6AE18A7 -->

Allows you to check if the minimap is currently expaned and/or if the full map is currently revealed on the minimap.

## Parameters
* **expandedRadar**: A pointer BOOL that gets set to true if the minimap is currently expanded.
* **revealFullMap**: A pointer BOOL that gets set to true if the full map is currently revealed on the minimap.

## Examples
```lua
local expanded, fullMap = GetBigmapActive()
print("The minimap is currently " .. (expanded and "expanded" or "normal size") .. " and the full map is currently " .. (fullMap and "revealed" or "not revealed") .. ".")
```
```c#
bool expanded = false;
bool fullMap = false;

GetBigmapActive(ref expanded, ref fullMap);

Debug.WriteLine($"The minimap is currently {(expanded ? "expanded" : "normal size")} and the full map is currently {(fullMap ? "revealed" : "not revealed")}.");
```
