---
ns: CFX
apiset: shared
---
## GET_INVOKING_RESOURCE

```c
char* GET_INVOKING_RESOURCE();
```


## Return value

Exaple:

AddEventHandler("script", function()
    print(GetInvokingResource()); -- If called from script1 will return Script1 else Script2
end)

--Script1
TriggerEvent("script") -- Returns Script1

--Script2
TriggerEvent("script") -- Returns Script2

Works only Client to Client and Server to Server
I think its working with exports too but im not sure
