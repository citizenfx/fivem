---
ns: CFX
apiset: server
---
## GET_PLAYER_ROUTING_BUCKET

```c
int GET_PLAYER_ROUTING_BUCKET(char* playerSrc);
```

Gets the routing bucket for the specified player.

Routing buckets are also known as 'dimensions' or 'virtual worlds' in past echoes, however they are population-aware.

## Parameters
* **playerSrc**: The player to get the routing bucket for.

## Return value
The routing bucket ID.
