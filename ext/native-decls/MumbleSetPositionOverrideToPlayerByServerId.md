---
ns: CFX
apiset: client
---
## MUMBLE_SET_POSITION_OVERRIDE_TO_PLAYER_BY_SERVER_ID

```c
void MUMBLE_SET_POSITION_OVERRIDE_TO_PLAYER_BY_SERVER_ID(int serverId, int sourceServerId, float range);
```

Plays a player's voice from another player's ped for the local client only, instead of from their own ped. The voice follows the source player as they move. This can be used for radio bleed, a phone on speaker or a body-worn radio.

While the override is set, `range` replaces the player's own talker proximity for hearing distance and falloff, and the player stays audible even when they are outside of the local player's scope. If the source player is outside of the local player's scope, the voice plays from the player's own position until the source player is back in scope. Only one position is supported per player, so this replaces any override set with [MUMBLE_SET_POSITION_OVERRIDE_BY_SERVER_ID](#_0xA32F280B). Volume overrides set with [MUMBLE_SET_VOLUME_OVERRIDE_BY_SERVER_ID](#_0xCE8E25B4) disable positional audio, so do not combine the two for the same player.

The override is applied on the next voice packet received from the player. Use [MUMBLE_CLEAR_POSITION_OVERRIDE_BY_SERVER_ID](#_0x291C4CE8) to remove it.

The local client must still receive the player's voice, for example by the talker adding the listener as a voice target.

## Parameters
* **serverId**: The server id of the player whose voice is moved.
* **sourceServerId**: The server id of the player to play the voice from.
* **range**: The hearing range in meters, must be greater than 0.
