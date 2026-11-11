---
ns: CFX
apiset: client
---
## MUMBLE_SET_POSITION_OVERRIDE_BY_SERVER_ID

```c
void MUMBLE_SET_POSITION_OVERRIDE_BY_SERVER_ID(int serverId, float x, float y, float z, float range);
```

Plays a player's voice from a fixed world position for the local client only, instead of from the player's ped. This can be used for speakers, intercoms or PA systems.

While the override is set, `range` replaces the player's own talker proximity for hearing distance and falloff, and the player stays audible even when they are outside of the local player's scope. Only one position is supported per player, so this replaces any override set with [MUMBLE_SET_POSITION_OVERRIDE_TO_PLAYER_BY_SERVER_ID](#_0x1F84414C). Volume overrides set with [MUMBLE_SET_VOLUME_OVERRIDE_BY_SERVER_ID](#_0xCE8E25B4) disable positional audio, so do not combine the two for the same player.

The override is applied on the next voice packet received from the player. Use [MUMBLE_CLEAR_POSITION_OVERRIDE_BY_SERVER_ID](#_0x291C4CE8) to remove it.

The local client must still receive the player's voice, for example by the talker adding the listener as a voice target.

## Parameters
* **serverId**: The player's server id.
* **x**: The X coordinate to play the voice from.
* **y**: The Y coordinate to play the voice from.
* **z**: The Z coordinate to play the voice from.
* **range**: The hearing range in meters, must be greater than 0.
