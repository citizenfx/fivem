---
ns: CFX
apiset: server
---
## IS_PLAYER_EVADING_WANTED_LEVEL

```c
BOOL IS_PLAYER_EVADING_WANTED_LEVEL(char* playerSrc);
```


## Parameters
* **playerSrc**: The target player

## Return value
Returns `true` if the player is evading wanted level, i.e. they are no longer in line of sights of police for a set amount of time
