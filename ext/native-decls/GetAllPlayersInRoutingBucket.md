+---
ns: CFX
apiset: server
---

## GET_ALL_PLAYERS_IN_ROUTING_BUCKET

```c
object GET_ALL_PLAYERS_IN_ROUTING_BUCKET(int bucket);
```

Gets all players that are in this routing bucket.
The data returned adheres to the following layout:
```
[127, 42, 13, 37]
```

Routing buckets are also known as 'dimensions' or 'virtual worlds' in past echoes, however they are population-aware.

## Parameters

- **bucket**: The routing bucket ID to get the players from.

## Return value

An object containing a list of player sources.

## Examples

```lua
local function SetPlayersInvincibleInBucket(bucket)
    local players = GetAllPlayersInRoutingBucket(bucket)

    for i = 1, #players do
        local player = players[i]
        SetPlayerInvincible(player, true)
    end
end
```

```js
const SetPlayersInvincibleInBucket = (bucket) => {
    const players = GetAllPlayersInRoutingBucket(bucket);

    for (let i = 0; i < players.length; i++) {
        const player = players[i];
        SetPlayerInvincible(player, true);
    }
}
```

```cs
using static CitizenFX.Core.Native.API;

private void SetPlayersInvincibleInBucket(int bucket) {
    const object players = GetAllPlayersInRoutingBucket(bucket);

    foreach (int player in players) {
        SetPlayerInvincible(player, true);
    }
}
```