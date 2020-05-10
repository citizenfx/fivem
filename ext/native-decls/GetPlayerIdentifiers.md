---
ns: CFX
apiset: server
---
## GET_PLAYER_IDENTIFIERS

```c
object GET_PLAYER_IDENTIFIERS(char* playerSrc);
```

## Parameters
* **playerSrc**: the player you want to get his identifiers list.

## Return value
Returns a table of all identifiers that a player has:
```
1 = steam
2 = license
3 = xbl
4 = live
5 = discord
6 = fivem
7 = ip
```
