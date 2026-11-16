---
ns: CFX
apiset: client
---
## SET_DISCORD_RICH_PRESENCE_PARTY

```c
void SET_DISCORD_RICH_PRESENCE_PARTY(char* partyId, int partySize, int partyMax);
```

Sets the party shown in the player's Discord rich presence, displayed as "(partySize of partyMax)" next to the state line.

The party is reset when disconnecting from a server. Pass an empty party ID and `0` for both sizes to clear it.

## Parameters
* **partyId**: An identifier for the party, such as a lobby or instance ID. Players sharing the same party ID are grouped together by Discord. Truncated to 127 characters.
* **partySize**: The current number of players in the party. Must not be negative or greater than `partyMax`.
* **partyMax**: The maximum number of players in the party. The size is only displayed when both `partySize` and `partyMax` are greater than `0`.

## Examples
```lua
SetDiscordRichPresenceParty('lobby-' .. lobbyId, #lobbyPlayers, 16)
```
