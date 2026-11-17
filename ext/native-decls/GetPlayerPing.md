---
ns: CFX
apiset: shared
---
## GET_PLAYER_PING

```c
int GET_PLAYER_PING(char* playerSrc);
```

See [GET_PLAYER_PEER_STATISTICS](#_0x9A928294) if you want more detailed information, like packet loss, and packet/rtt variance

On the client, only the local player is supported.

## Parameters
* **playerSrc**: The player to get the ping of. On the client, `nil`/`null` for the local player.

## Return value
Returns the mean amount of time a packet takes to get to the client, or -1 if the player has no connection.

## Examples
```lua
-- client
local ping = GetPlayerPing()
print(('ping: %dms'):format(ping))
```