---
ns: CFX
apiset: server
---
## DROP_PLAYER

```c
void DROP_PLAYER(char* playerSrc, char* reason);
```

Disconnects the player from the server and shows them the specified reason.

## Parameters
* **playerSrc**: The player to drop from the server
* **reason**: The reason to drop the player

```lua
DropPlayer(GetPlayers()[1], "Shouldn't have been the first!")
```
