---
ns: CFX
apiset: client
---
## MUMBLE_ADD_VOICE_CHANNEL_LISTEN

```c
void MUMBLE_ADD_VOICE_CHANNEL_LISTEN(int channel);
```

Starts listening to the specified mumble channel, if there is no channel this will silently fail.

## Parameters
* **channel**: A game voice channel ID.

## Examples
```lua
local TARGET_ID = 1
CreateThread(function()
    local cur_spectate_state = false
    while true do
        local is_spectating = NetworkIsInSpectatorMode()

        -- if we're not spectating then update the state
        if cur_spectate_state ~= is_spectating then
            cur_spectate_state = is_spectating
        else
            -- we don't want to spam add channel listens, so we'll skip subsequent calls
            goto already_spectating
        end

        if is_spectating then
            -- clear our voice targets so we can't talk to anyone
            MumbleClearVoiceTarget(TARGET_ID)

            local players = GetActivePlayers()
            for i = 1, #players do
                local ply = players[i]
                -- we need the players server id to get the voice channel
                local server_id = GetPlayerServerId(ply)
                -- tell mumble that we want to 
                MumbleAddVoiceChannelListen(MumbleGetVoiceChannelFromServerId(server_id))
            end
        end

        -- we'll jump to this if our spectating state is already up to date
        ::already_spectating::

        Wait(250)
    end
end)
```
