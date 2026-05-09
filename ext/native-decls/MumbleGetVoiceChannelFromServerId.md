---
ns: CFX
apiset: client
---
## MUMBLE_GET_VOICE_CHANNEL_FROM_SERVER_ID

```c
int MUMBLE_GET_VOICE_CHANNEL_FROM_SERVER_ID(int serverId);
```

## Parameters
* **serverId**: The player's server id.

## Return value
Returns the mumble voice channel from a player's server id, root channel will be `0`, returns `-1` if the client isn't connected.
