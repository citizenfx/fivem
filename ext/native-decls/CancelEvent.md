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

[Full Screen NUI guide for reference](https://docs.fivem.net/docs/scripting-manual/nui-development/full-screen-nui/)
