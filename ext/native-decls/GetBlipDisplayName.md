---
ns: CFX
apiset: client
game: gta5
---
## GET_BLIP_DISPLAY_NAME

```c
char* GET_BLIP_DISPLAY_NAME(Blip blip);
```

Reads the current name using GTA's internal blip-name function, including
names assigned by other resources. No registration by the creating resource
is required.

Returns a custom name when assigned, otherwise GTA's localized default name.
Formatting codes are preserved; render the result as text in NUI, not HTML.
The result does not include pause-map category headings or grouped counts.

## Parameters
* **blip**: A current blip handle.

## Return value
The name, or null for an invalid/deleted handle or unavailable implementation.
An empty string is possible when GTA supplies no name.

## Examples
```lua
if IsBlipDisplayNameAvailable() then
    local name = GetBlipDisplayName(blip)
end
```
