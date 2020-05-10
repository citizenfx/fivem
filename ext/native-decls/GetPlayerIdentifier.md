---
ns: CFX
apiset: server
---
## GET_PLAYER_IDENTIFIER

```c
char* GET_PLAYER_IDENTIFIER(char* playerSrc, int identifier);
```


## Parameters
* **playerSrc**: the player you want to get his identifier
* **identifier**: the identifier id:
```
0 = steam
1 = license
2 = xbl
3 = live
4 = discord
5 = fivem
6 = ip
```

## Return value
A string of the value requested.
