---
ns: CFX
apiset: shared
---
## ADD_STATE_BAG_CHANGE_HANDLER

```c
int ADD_STATE_BAG_CHANGE_HANDLER(char* keyFilter, char* bagFilter, func handler);
```

Adds a handler for changes to a state bag.

The function called expects to match the following signature:

```ts
function StateBagChangeHandler(bagName: string, key: string, value: any, reserved: number, replicated: boolean);
```

* **bagName**: The internal bag ID for the state bag which changed. This is usually `player:Source`, `entity:NetID`
  or `localEntity:Handle`.
* **key**: The changed key.
* **value**: The new value stored at key. The old value is still stored in the state bag at the time this callback executes.
* **reserved**: Currently unused.
* **replicated**: Whether the set is meant to be replicated.

At this time, the change handler can't opt to reject changes.

If bagName refers to an entity, use [GET_ENTITY_FROM_STATE_BAG_NAME](#_0x4BDF1867) to get the entity handle
If bagName refers to a player, use [GET_PLAYER_FROM_STATE_BAG_NAME](#_0xA56135E0) to get the player handle

## Parameters
* **keyFilter**: The key to check for, or null for no filter.
* **bagFilter**: The bag ID to check for such as `entity:65535`, or null for no filter.
* **handler**: The handler function.

## Return value
A cookie to remove the change handler.

## Examples
```js
AddStateBagChangeHandler("blockTasks", null, async (bagName, key, value /* boolean */) => {
    let entity = GetEntityFromStateBagName(bagName);
    // Whoops, we don't have a valid entity!
    if (entity === 0) return;
    // We don't want to freeze the entity position if the entity collision hasn't loaded yet
    while (!HasCollisionLoadedAroundEntity(entity)) {
        // The entity went out of our scope before the collision loaded
        if (!DoesEntityExist(entity)) return;
        await Delay(250);
    }
    SetEntityInvincible(entity, value)
    FreezeEntityPosition(entity, value)
    TaskSetBlockingOfNonTemporaryEvents(entity, value)
})
```

```lua
AddStateBagChangeHandler("blockTasks", nil, function(bagName, key, value) 
    local entity = GetEntityFromStateBagName(bagName)
    -- Whoops, we don't have a valid entity!
    if entity == 0 then return end
    -- We don't want to freeze the entity position if the entity collision hasn't loaded yet
    while not HasCollisionLoadedAroundEntity(entity) do
        -- The entity went out of our scope before the collision loaded
        if not DoesEntityExist(entity) then return end
        Wait(250)
    end
    SetEntityInvincible(entity, value)
    FreezeEntityPosition(entity, value)
    TaskSetBlockingOfNonTemporaryEvents(entity, value)
end)
```

```cs
AddStateBagChangeHandler("blockTasks", null,
    new Action<string, string, object, int, bool>
    (async (bagName, key, value, res, rep) =>
    {
        bool val = (bool)value;

        var entity = GetEntityFromStateBagName(bagName);
        //-- Whoops, we don't have a valid entity!
        if (entity == 0) return;
        //-- We don't want to freeze the entity position if the entity collision hasn't loaded yet
        while (!HasCollisionLoadedAroundEntity(entity))
        {
            //--The entity went out of our scope before the collision loaded
            if (!DoesEntityExist(entity)) return;

            await Delay(250);
        }
        SetEntityInvincible(entity, val);
        FreezeEntityPosition(entity, val);
        TaskSetBlockingOfNonTemporaryEvents(entity, val);
    }));
```
