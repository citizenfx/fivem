---
ns: CFX
apiset: client
game: gta5
---
## GET_PLAYER_INVINCIBLE_2

```c
BOOL GET_PLAYER_INVINCIBLE_2(Player player);
```
Unlike [GET_PLAYER_INVINCIBLE](#_0xB721981B2B939E07) this native gets both [SET_PLAYER_INVINCIBLE_KEEP_RAGDOLL_ENABLED](#_0x6BC97F4F4BB3C04B) and [SET_PLAYER_INVINCIBLE](#_0x239528EACDC3E7DE) invincibility state.

## Parameters
* **player**: The player id

## Return value
A boolean to tell if the player is invincible.
