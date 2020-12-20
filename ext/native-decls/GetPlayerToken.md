---
ns: CFX
apiset: server
---
## GET_PLAYER_TOKEN

```c
char* GET_PLAYER_TOKEN(char* playerSrc, int index);
```

Gets a player's token. Tokens can be used to enhance banning logic, however are specific to a server.

## Parameters
* **playerSrc**: A player.
* **index**: Index between 0 and GET_NUM_PLAYER_TOKENS.

## Return value
A token value.