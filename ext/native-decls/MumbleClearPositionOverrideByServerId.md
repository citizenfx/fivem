---
ns: CFX
apiset: client
---
## MUMBLE_CLEAR_POSITION_OVERRIDE_BY_SERVER_ID

```c
void MUMBLE_CLEAR_POSITION_OVERRIDE_BY_SERVER_ID(int serverId);
```

Removes a position override set with [MUMBLE_SET_POSITION_OVERRIDE_BY_SERVER_ID](#_0xA32F280B). The player's voice returns to their ped and their own talker proximity on the next voice packet.

## Parameters
* **serverId**: The player's server id.
