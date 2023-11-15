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
> LUA Example:
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
> C# Example:
```c#
using CitizenFX.Core;
using Newtonsoft.Json;

public class YourScript : BaseScript
{
    public YourScript()
    {
        EventHandlers["YourEventName"] += new Action<string>(OnNuiMessageReceived);
    }

    private void OnNuiMessageReceived(string jsonString)
    {
        dynamic data = JsonConvert.DeserializeObject(jsonString);

        if (data.action == "displaySomething")
        {
            string text = data.text;
            Debug.WriteLine($"Received NUI Message: {text}");
        }
    }

    private void SendNuiMessage()
    {
        dynamic data = new
        {
            action = "displaySomething",
            text = "Hello from C#!"
        };

        string jsonStr = JsonConvert.SerializeObject(data);

        Function.Call((Hash)0x7AE0589093A2E088, jsonStr); 
    }
}
```
> JS Example:
```js
function SendNuiMessage() {
    const data = {
        action: "displaySomething",
        text: "Hello world!"
    };

    const jsonStr = JSON.stringify(data);

    emit('SendNuiMessage', jsonStr);
}

on('YourEventName', (jsonString) => {
    const data = JSON.parse(jsonString);

    if (data.action === "displaySomething") {
        const text = data.text;
        console.log(`Received NUI Message: ${text}`);
    }
});

```
[Full Screen NUI guide for reference](https://docs.fivem.net/docs/scripting-manual/nui-development/full-screen-nui/)
