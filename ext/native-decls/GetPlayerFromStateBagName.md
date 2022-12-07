---
ns: CFX
apiset: shared 
---
## GET_PLAYER_FROM_STATE_BAG_NAME 

```c
int GET_PLAYER_FROM_STATE_BAG_NAME(char* bagName);
```
On the server this will return the players source, on the client it will return the player handle.

## Parameters
* **bagName**: An internal state bag ID from the argument to a state bag change handler.

## Return value
The player handle or 0 if the state bag name did not refer to a player, or the player does not exist.

## Examples
```js
AddStateBagChangeHandler("isDead", null, async (bagName, key, value /* boolean */) => {
    const ply = GetPlayerFromStateBagName(bagName);
    // The player doesn't exist!
    if (ply === 0) return;
    console.log(`Player: ${GetPlayerName(ply)} ${value ? 'died!' : 'is alive!'`)
})
```

```lua
AddStateBagChangeHandler("isDead", nil, function(bagName, key, value) 
    local ply = GetPlayerFromStateBagName(bagName)
    -- The player doesn't exist!
    if ply == 0 then return end
    print("Player: " .. GetPlayerName(ply) .. value and 'died!' or 'is alive!')
end)
```
