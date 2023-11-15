---
ns: CFX
apiset: client
---
## SEND_NUI_MESSAGE

```c
BOOL SEND_NUI_MESSAGE(char* jsonString);
```


## Parameters
* **jsonString**: 

## Return value

## Examples
```lua
local data = {
    action = "displaySomething",
    text = "Hello, World!"
}

local jsonStr = json.encode(data)

local result = SendNUIMessage(jsonStr)

if result then
    print("Message sent to NUI.")
else
    print("Failed to send message to NUI.")
end
```
[Full Screen NUI guide for reference](https://docs.fivem.net/docs/scripting-manual/nui-development/full-screen-nui/)
