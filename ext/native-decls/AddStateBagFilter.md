---
ns: CFX
apiset: server
---
## ADD_STATE_BAG_FILTER

```c
int ADD_STATE_BAG_FILTER(char* keyFilter, char* bagFilter, func handler);
```
**Experimental**: This native may be altered or removed in future versions of CitizenFX without warning.

Registers a state bag filter to handle rejection.

The state bag filter can reject replicated state bag changes by returning false in the handler, this will sync back the original value to the client and trigger the clients change handler.

**Note**: Every filter is not guaranteed to run, they run on the order which they where registered, until one of the filter rejects the value.

The function called expects to match the following signature:
```ts
function StateBagFilter(bagName: string, key: string, value: any): boolean;
```

* **bagName**: The internal bag ID for the state bag which changed. This is usually `player:Source`, `entity:NetID`
  or `localEntity:Handle`.
* **key**: The requested changed key.
* **value**: The value requested to be changed

If you want to get the caller of the filter, use [NETWORK_GET_ENTITY_OWNER](#_0x526FEE31)

If bagName refers to an entity, use [GET_ENTITY_FROM_STATE_BAG_NAME](?_0x4BDF1868) to get the entity handle
If bagName refers to a player, use [GET_PLAYER_FROM_STATE_BAG_NAME](?_0xA56135E0) to get the player handle

## Parameters
* **keyFilter**: The key to check for, or null for no filter.
* **bagFilter**: The bag ID to check for such as `entity:65535`, or null for no filter.
* **handler**: The handler function.

## Return value
A cookie to remove the filter.

## Examples
```js
const rejectStateChangeForNetIds = new Set([65535, 65538, 65577]);
AddStateBagFilter("blockTasks", null, async (bagName, key, value /* boolean */, source) => {
    let ent = GetEntityFromStateBagName(bagName);
    // Whoops, we were don't have a valid entity!
    if (ent === 0) return;
    const netId = NetworkGetNetworkIdFromEntity(entity);
    if (rejectStateChangeForNetIds.has(netId) || typeof (value) !== "boolean") {
        // Reject the change
        return false;
    }
    // Accept the change
    return true;
})
```

```lua
local rejectStateChangeForNetIds = { [65535] = true, [65538] = true, [65577] = true }
AddStateBagFilter("blockTasks", nil, function(bagName, key, value)
    local ent = GetEntityFromStateBagName(bagName)
    -- Whoops, we don't have a valid entity!
    if ent == 0 then return end
    local netId = NetworkGetNetworkIdFromEntity(entity);
    if rejectStateChangeForNetIds[netId] or type(value) ~= "number" then
        -- Reject the change
        return false
    end
    -- Accept the change
    return true
end)
```

