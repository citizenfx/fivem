---
ns: CFX
apiset: shared
---
## GET_INVOKING_RESOURCE

```c
char* GET_INVOKING_RESOURCE();
```


## Example

```lua
AddEventHandler("event", function(cb)
    cb(GetInvokingResource())
end)

--Script1
TriggerEvent("event", function(resource) 
    print(resource) -- Will print Script1
end)

--Script2
TriggerEvent("event", function(resource) 
    print(resource) -- Will print Script2
end)
```
