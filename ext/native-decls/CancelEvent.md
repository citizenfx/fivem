---
ns: CFX
apiset: shared
---
## CANCEL_EVENT

```c
void CANCEL_EVENT();
```

Cancels the currently executing event.

## Examples
```lua
    AddEventHandler('ptFxEvent', function(sender, eventName, eventData)
    CancelEvent()
end)
```
