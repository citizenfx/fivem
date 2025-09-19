---
ns: CFX
apiset: shared
---
## WAS_EVENT_CANCELED

```c
BOOL WAS_EVENT_CANCELED();
```

## Return value
Returns `true` if the event was canceled, `false` otherwise.

## Examples
```lua
--- This code would be thought to be in a different resource
local currentActionId = {}
AddEventListener("canPlayerDoAction", function(source, actionId)
    if currentActionId[source] == actionId then
        -- woah! they're already doing this action, don't let them do it again.
        CancelEvent()
    end
    currentActionId[source] = actionId
end)

RegisterNetEvent("onAction", function(actionId)
    -- check if other resources are fine with the player doing the action
    TriggerEvent("canPlayerDoAction", source, actionId)
    -- resource was not fine with it, don't do anything else with the event.
    if WasEventCanceled() then
        return
    end
end)
```
