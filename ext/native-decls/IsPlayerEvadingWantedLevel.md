---
ns: CFX
apiset: server
---
## IS_PLAYER_EVADING_WANTED_LEVEL

```c
BOOL IS_PLAYER_EVADING_WANTED_LEVEL(char* playerSrc);
```

```
This will return true if the player is evading wanted level, meaning that the wanted level stars are blink.
Otherwise will return false.

If the player is not wanted, it simply returns false.
```

## Parameters
* **playerSrc**: The target player

## Return value
boolean value, depending if the player is evading his wanted level or not.
