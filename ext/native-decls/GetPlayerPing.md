---
ns: CFX
apiset: server
---
## GET_PLAYER_PING

```c
int GET_PLAYER_PING(char* playerSrc);
```

See [GET_PLAYER_PEER_STATISTICS](#_0x9A928294) if you want more detailed information, like packet loss, and packet/rtt variance

## Parameters
* **playerSrc**: The player to get the ping of

## Return value
Returns the mean amount of time a packet takes to get to the client
