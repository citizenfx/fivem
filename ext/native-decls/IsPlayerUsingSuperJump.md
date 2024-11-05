---
ns: CFX
apiset: server
---
## IS_PLAYER_USING_SUPER_JUMP

```c
BOOL IS_PLAYER_USING_SUPER_JUMP(char* playerSrc);
```

Server getter for client native [SET_SUPER_JUMP_THIS_FRAME](#_0x57FFF03E423A4C0B)

## Parameters
* **playerSrc**: The player handle

## Return value
Returns `true` if the player has super jump enabled.
