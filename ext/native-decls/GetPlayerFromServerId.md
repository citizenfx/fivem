---
ns: CFX
apiset: client
---
## GET_PLAYER_FROM_SERVER_ID

```c
Player GET_PLAYER_FROM_SERVER_ID(int serverId);
```

Gets a local client's Player ID from its server ID counterpart, assuming the passed `serverId` exists on the client.

If no matching client is found, or an invalid value is passed over as the `serverId` native's parameter, the native result will be `-1`.

It's worth noting that this native method can only retrieve information about clients that are culled to the connected client.

## Parameters
* **serverId**: The player's server ID.

## Return value
A valid Player ID if one is found, `-1` if not.


## Examples
```lua
--We will assume the serverId is '4' in this scenario and that it's a valid serverId.

-- Passing invalid Player IDs such as 'nil' or IDs that don't exist will result in playerId being -1.

local playerId = GetPlayerFromServerId(serverId);

-- If the resulting playerId is not invalid (not equal to -1)
if playerId ~= -1 then
    -- Do our stuff on this player.
end

```