---
ns: CFX
apiset: server
---
## GET_PLAYER_ENDPOINT

```c
char* GET_PLAYER_ENDPOINT(char* playerSrc);
```
Gets a specified player's IP address.
This example will print the players IP as soon as they connect.
```lua
AddEventHandler('playerConnecting', function(playerName, setKickReason, deferrals)
    local ipAddress = GetPlayerEndpoint(source)
    print(ipAddress)
end)
```

## Parameters
* **playerSrc**: The player, who's IP we want to get.

## Return value
A players IP.
