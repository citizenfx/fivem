---
ns: CFX
apiset: client
---
## SET_DISCORD_RICH_PRESENCE_TIMESTAMPS

```c
void SET_DISCORD_RICH_PRESENCE_TIMESTAMPS(long startTimestamp, long endTimestamp);
```

Sets the timer shown in the player's Discord rich presence.

When only a start timestamp is set, Discord shows the time elapsed since then. When an end timestamp is set, Discord shows the time remaining until then instead.

The timestamps are reset when disconnecting from a server.

## Parameters
* **startTimestamp**: A Unix timestamp in seconds, such as the start of the current match. Pass `0` to keep the default, which is the time the player joined the server.
* **endTimestamp**: A Unix timestamp in seconds, such as the end of the current round. Pass `0` to show no countdown. Must not be earlier than `startTimestamp` when both are set.

## Examples
```lua
local now = GetCloudTimeAsInt()
SetDiscordRichPresenceTimestamps(now, now + roundDurationSeconds)
```
