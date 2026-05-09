---
ns: CFX
apiset: server
---
## GET_NUM_PLAYER_IDENTIFIERS

```c
int GET_NUM_PLAYER_IDENTIFIERS(char* playerSrc);
```


## Parameters
* **playerSrc**: The player to get the number identifiers of

## Return value
Returns the number of identifiers that the player has

## Examples
```lua
function GetPlayerIdentifiers(player)
    local numIds = GetNumPlayerIdentifiers(player)
    local t = {}

    for i = 0, numIds - 1 do
        table.insert(t, GetPlayerIdentifier(player, i))
    end

    return t
end
```
