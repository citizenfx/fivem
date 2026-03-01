---
ns: CFX
apiset: client
---
## ADD_TEXT_ENTRY

```c
void ADD_TEXT_ENTRY(char* entryKey, char* entryText);
```


## Parameters
* **entryKey**: 
* **entryText**: 

## Examples
```lua
local key_names = {
    ["TEXT_KEY_1"] = "Text Key 1",
    ["TEXT_KEY_2"] = "Text Key 2",
    ["TEXT_KEY_3"] = "Text Key 3",
    ["TEXT_KEY_4"] = "Text Key 4",
}

CreateThread(function()
    for k,v in pairs(key_names) do
        AddTextEntry(k, v)
    end
end)
```