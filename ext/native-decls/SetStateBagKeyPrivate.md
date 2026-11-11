---
ns: CFX
apiset: server
---
## SET_STATE_BAG_KEY_PRIVATE

```c
BOOL SET_STATE_BAG_KEY_PRIVATE(char* bagName, char* keyName, BOOL isPrivate);
```

Marks a state bag key as private, or makes it public again.

A private key is only replicated between the server and the owner of the state bag: the player for a `player:` bag, and the current entity owner for an `entity:` bag. Other players never receive it, whether they are in scope already or enter it later. The `global` bag has no owner, so a private key in it stays on the server.

The mode is stored per key and applies to every later write, including writes by the owner. It lasts as long as the state bag itself, so it has to be set again for a new player or entity.

If the key already has a value, marking it private removes that value for everyone except the owner, and making it public again sends the current value to everyone the state bag replicates to. When an entity changes owner, the new owner receives its private keys and the previous owner loses them.

## Parameters
* **bagName**: The name of the bag, such as `player:1`, `entity:5` or `global`.
* **keyName**: The key to change the mode of.
* **isPrivate**: `true` to only replicate the key to the owner of the state bag, `false` to replicate it to everyone again.

## Return value
Whether the state bag exists.

## Examples
```lua
-- only the player themselves and the server can read the balance
local function setBalance(playerSrc, amount)
    SetStateBagKeyPrivate(('player:%s'):format(playerSrc), 'balance', true)
    Player(playerSrc).state.balance = amount
end
```
