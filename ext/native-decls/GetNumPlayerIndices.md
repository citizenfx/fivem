---
ns: CFX
apiset: server
---
## GET_NUM_PLAYER_INDICES

```c
int GET_NUM_PLAYER_INDICES();
```

## Return value
Returns the number of players that are currently in the server

## Examples
```lua
-- example from scheduler
function GetPlayers()
    local num = GetNumPlayerIndices()
    local t = {}

    for i = 0, num - 1 do
        table.insert(t, GetPlayerFromIndex(i))
    end

    return t
end
```
