---
ns: CFX
apiset: shared
---
## ADD_STATE_BAG_CHANGE_HANDLER

```c
int ADD_STATE_BAG_CHANGE_HANDLER(char* keyFilter, char* bagFilter, func handler);
```

Registers a `StateBagChangeHandler` for the specified filters that you want to listen to.

The function will be called with the following signature:
```ts
function StateBagChangeHandler(bagName: string, key: string, value: any, reserved: number, replicated: boolean);
```

* **bagName**: The string representation bag ID for the state bag which changed. This is usually `global`, `player:Source`, `entity:NetID`
  or `localEntity:Handle`.
* **key**: The key that was changed
* **value**: The new value that will be stored on the state bag `key`
* **reserved**: Currently unused.
* **replicated**: Whether the change is meant to be replicated.

You can use either [GET_ENTITY_FROM_STATE_BAG_NAME](#_0x4BDF1867) or [GET_PLAYER_FROM_STATE_BAG_NAME](#_0xA56135E0) to get the entit/playery handle from the state bag.

**NOTE**: Whenever using state bag change handlers you should always use the `value` field from the callback, not the `Entity(id).state[key]` or `GlobalState[key]` values.  The values in the state bag will be the old value until the change handler finishes running.

## Parameters
* **keyFilter**: The key to check for, or null for no filter.
* **bagFilter**: The bag ID to check for such as `entity:65535`, `player:123`, `global`, or null for no filter.
* **handler**: The function that will be called whenever the state bag gets changed

## Return value
The cookie that can be used to remove the state bag change handler with [REMOVE_STATE_BAG_CHANGE_HANDLER](#_0xD36BE661)

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
