---
ns: CFX
apiset: client
---
## ADD_TEXT_ENTRY_BY_HASH

```c
void ADD_TEXT_ENTRY_BY_HASH(Hash entryKey, char* entryText);
```


## Parameters
* **entryKey**: 
* **entryText**: 

## Examples
```lua
local key_names = {
    [0x9806FA48] = "Text Key 1",
    [0x65C895CC] = "Text Key 2",
    [0x1CA48385] = "Text Key 3",
    [0x8A6E5F17] = "Text Key 4",
}

CreateThread(function()
    for k,v in pairs(key_names) do
        AddTextEntryByHash(k, v)
    end
end)
```