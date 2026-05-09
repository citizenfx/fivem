---
ns: CFX
apiset: client
---
## GET_ACTIVE_PLAYERS

```c
object GET_ACTIVE_PLAYERS();
```

Returns all player indices for 'active' physical players known to the client.
The data returned adheres to the following layout:
```
[127, 42, 13, 37]
```

## Return value
An object containing a list of player indices.

## Examples
```lua
local players = GetActivePlayers()
for i = 1, #players do
    local player = players[i]
    local playerName = GetPlayerName(player)

    print("player", player, "has the name:", playerName)
end
```
