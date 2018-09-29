---
ns: CFX
apiset: client
---
## SET_PLAYER_TALKING_OVERRIDE

```c
void SET_PLAYER_TALKING_OVERRIDE(Player player, BOOL state);
```

the status of default voip system. It affects on `NETWORK_IS_PLAYER_TALKING` and `mp_facial` animation.
This function doesn't need to be called every frame, it works like a switcher.

## Parameters
* **player**: The target player.
* **state**: Overriding state.

