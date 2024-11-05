---
ns: CFX
apiset: server
---
## GET_PLAYER_NAME

```c
char* GET_PLAYER_NAME(char* playerSrc);
```


## Parameters
* **playerSrc**: The player to get the name of

## Return value
Returns the players name as sent by the client, this will not be escaped.
