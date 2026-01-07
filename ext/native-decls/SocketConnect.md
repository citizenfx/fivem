---
ns: CFX
apiset: client
---
## SOCKET_CONNECT

```c
int SOCKET_CONNECT(char* url, func onEvent);
```

Connects to a WebSocket server at the specified URL.

This is a client-side only function that allows script resources to establish WebSocket connections to external servers. Multiple connections can be managed simultaneously.

The `onEvent` callback is triggered for all WebSocket events:
- "open" - Connection established
- "message" - Message received (data contains the message)
- "close" - Connection closed
- "error" - Error occurred (data contains error message)

## Parameters
* **url**: The WebSocket URL to connect to (must start with "ws://" or "wss://")
* **onEvent**: A callback function that receives two parameters: eventType (string) and data (string)

## Return value
Returns a socket ID (positive integer) on success, or -1 on failure.

## Examples

```lua
-- Connect to WebSocket server with a general event handler
local socketId = SocketConnect("ws://localhost:8080", function(eventType, data)
    print("WebSocket event: " .. eventType)
    if eventType == "message" then
        print("Received: " .. data)
    elseif eventType == "error" then
        print("Error: " .. data)
    end
end)

if socketId > 0 then
    print("Connected with socket ID: " .. socketId)
else
    print("Failed to connect")
end
```

```js
// JavaScript example
const socketId = SocketConnect("ws://localhost:8080", (eventType, data) => {
    console.log(`WebSocket event: ${eventType}`);
    if (eventType === "message") {
        console.log(`Received: ${data}`);
    }
});
```

```cs
// C# example
int socketId = Function.Call<int>(Hash.SOCKET_CONNECT, "ws://localhost:8080", new Action<string, string>((eventType, data) =>
{
    Debug.WriteLine($"WebSocket event: {eventType}, data: {data}");
}));
```
