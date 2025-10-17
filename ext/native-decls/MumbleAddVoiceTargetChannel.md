---
ns: CFX
apiset: client
---
## MUMBLE_ADD_VOICE_TARGET_CHANNEL

```c
void MUMBLE_ADD_VOICE_TARGET_CHANNEL(int targetId, int channel);
```

Adds the specified channel to the target list for the specified Mumble voice target ID.

## Parameters
* **targetId**: A Mumble voice target ID, ranging from 1..30 (inclusive).
* **channel**: A game voice channel ID.

## Examples
```lua
local TARGET_ID = 1

-- remove our current voice targets
MumbleClearVoiceTarget(TARGET_ID)

local players = GetActivePlayers()
for i = 1, #players do
    local ply = players[i]
    -- we need the players server id to use this
    local server_id = GetPlayerServerId(ply)
    -- tell mumble that we want to target the players current channel so they can hear us.
    MumbleAddVoiceTargetChannel(TARGET_ID, MumbleGetVoiceChannelFromServerId(server_id))
end
```
