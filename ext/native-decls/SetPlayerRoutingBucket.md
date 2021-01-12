---
ns: CFX
apiset: server
---
## SET_PLAYER_ROUTING_BUCKET

```c
void SET_PLAYER_ROUTING_BUCKET(char* playerSrc, int bucket);
```

Sets the routing bucket for the specified player.

Routing buckets are also known as 'dimensions' or 'virtual worlds' in past echoes, however they are population-aware.

## Parameters
* **playerSrc**: The player to set the routing bucket for.
* **bucket**: The bucket ID.
