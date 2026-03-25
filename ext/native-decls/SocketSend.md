---
ns: CFX
apiset: client
---
## SOCKET_SEND

```c
BOOL SOCKET_SEND(int socketId, char* data);
```

Sends data through a WebSocket connection.

The data is sent as a text frame. The connection must be in the "open" state for the send to succeed.

## Parameters
* **socketId**: The socket ID returned by `SOCKET_CONNECT`
* **data**: The string data to send through the WebSocket

## Return value
Returns `true` if the data was sent successfully, `false` if the socket was not found or not in an open state.

## Examples

```lua
-- Connect and send data
local socketId = SocketConnect("ws://localhost:8080", function(eventType, data)
    if eventType == "message" then
        print("Server response: " .. data)
    end
end)

-- Register open handler to send initial message
SocketOn(socketId, "open", function()
    -- Send a message to the server
    if SocketSend(socketId, "Hello Server!") then
        print("Message sent successfully")
    else
        print("Failed to send message")
    end

    -- Send JSON data
    local jsonData = json.encode({ action = "ping", timestamp = GetGameTimer() })
    SocketSend(socketId, jsonData)
end)
```

```js
// JavaScript example
const socketId = SocketConnect("ws://localhost:8080", (eventType, data) => {
    if (eventType === "message") {
        console.log(`Server response: ${data}`);
    }
});

SocketOn(socketId, "open", () => {
    // Send a message
    SocketSend(socketId, "Hello Server!");

    // Send JSON data
    SocketSend(socketId, JSON.stringify({ action: "ping" }));
});
```

```cs
// C# example
int socketId = Function.Call<int>(Hash.SOCKET_CONNECT, "ws://localhost:8080", new Action<string, string>((e, d) => { }));

Function.Call<bool>(Hash.SOCKET_ON, socketId, "open", new Action<string>((_) =>
{
    bool sent = Function.Call<bool>(Hash.SOCKET_SEND, socketId, "Hello Server!");
    Debug.WriteLine($"Message sent: {sent}");
}));
```
