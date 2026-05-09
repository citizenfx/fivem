---
ns: CFX
apiset: server
---
## GET_NUM_PLAYER_TOKENS

```c
int GET_NUM_PLAYER_TOKENS(char* playerSrc);
```


## Parameters
* **playerSrc**: 

## Return value
Returns the amount of tokens the current player has

## Examples
```lua
function GetPlayerTokens(player)
    local numIds = GetNumPlayerTokens(player)
    local t = {}

    for i = 0, numIds - 1 do
        table.insert(t, GetPlayerToken(player, i))
    end

    return t
end
```
