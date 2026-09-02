---
ns: CFX
apiset: client
---
## SOCKET_ON

```c
BOOL SOCKET_ON(int socketId, char* eventType, func handler);
```

Registers an event listener for specific WebSocket events on a socket.

This allows you to register multiple handlers for specific event types, in addition to the general `onEvent` callback provided to `SOCKET_CONNECT`.

Supported event types:
- "open" - Handler receives no data (empty string)
- "message" - Handler receives the message content
- "close" - Handler receives no data (empty string)
- "error" - Handler receives the error message

Multiple handlers can be registered for the same event type on the same socket.

## Parameters
* **socketId**: The socket ID returned by `SOCKET_CONNECT`
* **eventType**: The event type to listen for ("open", "message", "close", "error")
* **handler**: A callback function that receives one parameter: data (string)

## Return value
Returns `true` if the handler was registered successfully, `false` if the socket ID was not found.

## Examples

```lua
-- Connect to WebSocket server
local socketId = SocketConnect("ws://localhost:8080", function(eventType, data)
    -- General event handler
end)

-- Register specific event handlers
SocketOn(socketId, "open", function()
    print("Connected!")
    SocketSend(socketId, "Hello Server")
end)

SocketOn(socketId, "message", function(message)
    print("Received: " .. message)
end)

SocketOn(socketId, "error", function(error)
    print("Error: " .. error)
end)

SocketOn(socketId, "close", function()
    print("Connection closed")
end)
```

```js
// JavaScript example
const socketId = SocketConnect("ws://localhost:8080", () => {});

SocketOn(socketId, "open", () => {
    console.log("Connected!");
    SocketSend(socketId, "Hello Server");
});

SocketOn(socketId, "message", (message) => {
    console.log(`Received: ${message}`);
});

SocketOn(socketId, "error", (error) => {
    console.log(`Error: ${error}`);
});
```

```cs
// C# example
int socketId = Function.Call<int>(Hash.SOCKET_CONNECT, "ws://localhost:8080", new Action<string, string>((e, d) => { }));

Function.Call<bool>(Hash.SOCKET_ON, socketId, "message", new Action<string>((message) =>
{
    Debug.WriteLine($"Received: {message}");
}));
```
